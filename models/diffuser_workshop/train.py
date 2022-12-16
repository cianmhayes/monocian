import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))


from datasets import load_dataset
from diffusers import DDIMPipeline, DDPMScheduler, DDIMScheduler, UNet2DModel
from dotenv import load_dotenv
import torch
import torch.nn.functional as F
import torchvision
from torchvision import transforms
from common.datasets.azure_container_image_dataset import cache_blobs
import wandb
from PIL import Image
import numpy as np
from tqdm import tqdm
import random
from datetime import datetime


def make_data_loader(dataset, image_mode="RGB", image_size=32, batch_size=64):
    # Define data augmentations
    preprocess = transforms.Compose(
        [
            transforms.Resize((image_size, image_size)),  # Resize
            transforms.RandomHorizontalFlip(),  # Randomly flip (data augmentation)
            transforms.ToTensor(),  # Convert to tensor (0, 1)
            transforms.Normalize([0.5], [0.5]),  # Map to (-1, 1)
        ]
    )

    def transform(examples):
        images = [preprocess(image.convert(image_mode)) for image in examples["image"]]
        return {"images": images}

    dataset.set_transform(transform)

    # Create a dataloader from the dataset to serve up the transformed images in batches
    return torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=True)


def make_model(image_size: int, channels: int, device):
    # Create a model
    model = UNet2DModel(
        sample_size=image_size,  # the target image resolution
        in_channels=channels,  # the number of input channels, 3 for RGB images
        out_channels=channels,  # the number of output channels
        layers_per_block=2,  # how many ResNet layers to use per UNet block
        block_out_channels=(64, 128, 128, 256),  # More channels -> more parameters
        down_block_types=(
            "DownBlock2D",  # a regular ResNet downsampling block
            "DownBlock2D",
            "AttnDownBlock2D",  # a ResNet downsampling block with spatial self-attention
            "AttnDownBlock2D",
        ),
        up_block_types=(
            "AttnUpBlock2D",
            "AttnUpBlock2D",  # a ResNet upsampling block with spatial self-attention
            "UpBlock2D",
            "UpBlock2D",  # a regular ResNet upsampling block
        ),
    )
    return model.to(device)


class Trainer(object):
    def __init__(
        self,
        model,
        train_dataset,
        test_dataset,
        validation_dataset,
        image_mode: str,
        image_size: int,
        batch_size: int,
        gradient_accumulation_steps: int,
    ) -> None:
        self.model = model
        self.device = model.device
        self.train_dataloader = make_data_loader(
            train_dataset, image_mode, image_size, batch_size
        )
        self.test_dataloader = make_data_loader(
            test_dataset, image_mode, image_size, batch_size
        )
        self.validation_dataloader = make_data_loader(
            validation_dataset, image_mode, image_size, batch_size
        )
        self.channels = len(image_mode)
        self.image_size = image_size
        self.batch_size = batch_size
        self.gradient_accumulation_steps = gradient_accumulation_steps
        self.noise_scheduler = DDPMScheduler(
            num_train_timesteps=1000, beta_schedule="squaredcos_cap_v2"
        )

        self.sampling_scheduler = DDIMScheduler.from_config(self.noise_scheduler.config)
        self.sampling_scheduler.set_timesteps(num_inference_steps=50)

        self.optimizer = torch.optim.AdamW(self.model.parameters(), lr=0.001)
        self.lr_scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
            self.optimizer, 64, eta_min=0.00001
        )

        self.model_save_name = os.path.join(
            ".",
            "output",
            datetime.now().strftime("%Y%m%d_%H%M%S"),
            "saved_models",
            "diffuser_experiment_",
        )

    def _train_phase(self) -> float:
        self.model.train()
        losses = []
        pending_optimizer_step = False
        for step, batch in tqdm(enumerate(self.train_dataloader)):
            clean_images = batch["images"].to(device)
            # Sample noise to add to the images
            noise = torch.randn(clean_images.shape).to(clean_images.device)
            bs = clean_images.shape[0]

            # Sample a random timestep for each image
            timesteps = torch.randint(
                0,
                self.noise_scheduler.num_train_timesteps,
                (bs,),
                device=clean_images.device,
            ).long()

            # Add noise to the clean images according to the noise magnitude at each timestep
            noisy_images = self.noise_scheduler.add_noise(
                clean_images, noise, timesteps
            )

            # Get the model prediction
            noise_pred = self.model(noisy_images, timesteps, return_dict=False)[0]

            # Calculate the loss
            loss = F.mse_loss(noise_pred, noise)
            loss.backward(loss)
            losses.append(loss.item())
            pending_optimizer_step = True
            wandb.log(
                {"training_loss": loss, "lr": float(self.lr_scheduler.get_last_lr()[0])}
            )

            # Update the model parameters with the optimizer
            # Gradient accumulation:
            if (step + 1) % self.gradient_accumulation_steps == 0:
                pending_optimizer_step = False
                self.optimizer.step()
                self.optimizer.zero_grad()
                self.lr_scheduler.step()

        if pending_optimizer_step:
            self.optimizer.step()
            self.optimizer.zero_grad()
            self.lr_scheduler.step()
        return sum(losses) / len(self.train_dataloader)

    def _eval_phase(self, dataloader) -> float:
        with torch.no_grad():
            self.model.eval()
            losses = []
            for _, batch in tqdm(enumerate(dataloader)):
                clean_images = batch["images"].to(device)
                # Sample noise to add to the images
                noise = torch.randn(clean_images.shape).to(clean_images.device)
                bs = clean_images.shape[0]
                # Sample a random timestep for each image
                timesteps = torch.randint(
                    0,
                    self.noise_scheduler.num_train_timesteps,
                    (bs,),
                    device=clean_images.device,
                ).long()
                # Add noise to the clean images according to the noise magnitude at each timestep
                noisy_images = self.noise_scheduler.add_noise(
                    clean_images, noise, timesteps
                )
                # Get the model prediction
                noise_pred = self.model(noisy_images, timesteps, return_dict=False)[0]
                # Calculate the loss
                loss = F.mse_loss(noise_pred, noise)
                losses.append(loss.item())
            return sum(losses) / len(dataloader)

    def _log_samples(self) -> None:
        x = torch.randn(8, self.channels, self.image_size, self.image_size).to(
            self.device
        )  # Batch of 8
        for i, t in tqdm(enumerate(self.sampling_scheduler.timesteps)):
            model_input = self.sampling_scheduler.scale_model_input(x, t)
            with torch.no_grad():
                noise_pred = model(model_input, t)["sample"]
            x = self.sampling_scheduler.step(noise_pred, t, x).prev_sample
        grid = torchvision.utils.make_grid(x, nrow=4)
        im = grid.permute(1, 2, 0).cpu().clip(-1, 1) * 0.5 + 0.5
        im = Image.fromarray(np.array(im * 255).astype(np.uint8))
        wandb.log({"Sample generations": wandb.Image(im)})

    def _save_model(self, epoch):
        DDIMPipeline(model, self.sampling_scheduler).save_pretrained(
            self.model_save_name + f"step_{epoch+1}"
        )

    def train(self, epochs):
        lowest_train_loss = None
        lowest_validation_loss = None
        for epoch in range(epochs):
            print("============================================================")
            print(f" Epoch {epoch+1}")
            print("============================================================")
            should_save_model = False
            metrics = {}
            epoch_train_loss = self._train_phase()
            metrics["Epoch-end Train Loss"] = epoch_train_loss
            print(f"Training loss: {epoch_train_loss:0.4f}")
            if lowest_train_loss is None or epoch_train_loss < lowest_train_loss:
                should_save_model = True
            if (
                self.validation_dataloader is not None
                and len(self.validation_dataloader) > 0
            ):
                validation_loss = self._eval_phase(self.validation_dataloader)
                metrics["Epoch-end Validation Loss"] = validation_loss
                print(f"Validation loss: {validation_loss:0.4f}")
                if (
                    lowest_validation_loss is None
                    or validation_loss < lowest_validation_loss
                ):
                    should_save_model = True
            if self.test_dataloader is not None and len(self.test_dataloader) > 0:
                test_loss = self._eval_phase(self.test_dataloader)
                print(f"Test loss: {test_loss:0.4f}")
                metrics["Epoch-end Test Loss"] = test_loss
            print("Logging")
            wandb.log(metrics)
            self._log_samples()
            if should_save_model:
                self._save_model(epoch)


if __name__ == "__main__":
    load_dotenv()
    random.seed(20221216)
    root = os.path.join(os.path.dirname(__file__), "tmp", "dataset")
    if not os.path.exists(root):
        os.makedirs(root)
    cache_blobs(
        root,
        os.environ.get("AZURE_BLOB_CONNECTION_STR"),
        os.environ.get("AZURE_BLOB_CONTAINER_NAME"),
        0.1,
        0.1,
        512,
    )
    wandb.init(project="diffusers experiment", entity="cian-m-hayes")
    image_mode = "L"
    channels = len(image_mode)
    image_size = 128
    batch_size = 4
    gradient_accumulation_steps = 16
    epochs = 1000
    train_dataset = load_dataset("imagefolder", data_dir=root, split="train")
    test_dataset = load_dataset("imagefolder", data_dir=root, split="test")
    validation_dataset = load_dataset("imagefolder", data_dir=root, split="validation")
    # dataloader = make_data_loader(dataset, image_mode, image_size, 4)
    device = torch.device("cpu")
    if torch.cuda.is_available():
        device = torch.device("cuda")
        print("Using cuda")
    model = make_model(image_size, channels, device)
    trainer = Trainer(
        model,
        train_dataset,
        test_dataset,
        validation_dataset,
        image_mode,
        image_size,
        batch_size,
        gradient_accumulation_steps,
    )
    trainer.train(epochs)
