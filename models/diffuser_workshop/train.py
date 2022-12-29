import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import numpy as np
import random
import torch
import torch.nn.functional as F
import torchvision
import wandb
from common.datasets.azure_container_image_dataset import cache_blobs
from common.trainers.basic_trainer import BasicTrainer
from common.trainers.dataloader_manager import DataloaderManager
from datasets import load_dataset
from datetime import datetime
from diffusers import DDIMPipeline, DDPMScheduler, DDIMScheduler, UNet2DModel
from dotenv import load_dotenv
from PIL import Image
from torch.optim import Optimizer
from torch.optim.lr_scheduler import _LRScheduler
from torch.utils.data import DataLoader
from torchvision import transforms
from tqdm import tqdm
from typing import Dict


class SimpleDataloaderManager(DataloaderManager):
    def __init__(
        self, path: str, image_mode: str, image_size: int, batch_size: int
    ) -> None:
        super().__init__()
        self.train_dataset = load_dataset("imagefolder", data_dir=path, split="train")
        self.test_dataset = load_dataset("imagefolder", data_dir=path, split="test")
        self.validation_dataset = load_dataset(
            "imagefolder", data_dir=path, split="validation"
        )
        self.train_dataloader = self._make_data_loader(
            self.train_dataset, image_mode, image_size, batch_size
        )
        self.test_dataloader = self._make_data_loader(
            self.test_dataset, image_mode, image_size, batch_size
        )
        self.validation_dataloader = self._make_data_loader(
            self.validation_dataset, image_mode, image_size, batch_size
        )

    def _make_data_loader(self, dataset, image_mode, image_size, batch_size):
        preprocess = transforms.Compose(
            [
                transforms.Resize((image_size, image_size)),  # Resize
                transforms.RandomHorizontalFlip(),  # Randomly flip (data augmentation)
                transforms.ToTensor(),  # Convert to tensor (0, 1)
                transforms.Normalize([0.5], [0.5]),  # Map to (-1, 1)
            ]
        )

        def transform(examples):
            images = [
                preprocess(image.convert(image_mode)) for image in examples["image"]
            ]
            return {"images": images}

        dataset.set_transform(transform)
        return torch.utils.data.DataLoader(dataset, batch_size=batch_size, shuffle=True)

    def get_testing_dataloader(self) -> DataLoader:
        return self.train_dataloader

    def get_training_dataloader(self) -> DataLoader:
        return self.train_dataloader

    def get_validation_dataloader(self) -> DataLoader:
        return self.validation_dataloader


class DiffuserTrainer(BasicTrainer):
    def __init__(
        self,
        model: torch.nn.Module,
        dataloader_manager: DataloaderManager,
        optimizer: Optimizer,
        lr_scheduler: _LRScheduler = None,
        gradient_accumulation_steps: int = 1,
    ) -> None:
        super().__init__(
            model,
            dataloader_manager,
            optimizer,
            lr_scheduler,
            gradient_accumulation_steps,
        )
        self.noise_scheduler = DDPMScheduler(
            num_train_timesteps=1000, beta_schedule="squaredcos_cap_v2"
        )

        self.sampling_scheduler = DDIMScheduler.from_config(self.noise_scheduler.config)
        self.sampling_scheduler.set_timesteps(num_inference_steps=50)

        self.model_save_name = os.path.join(
            ".",
            "output",
            datetime.now().strftime("%Y%m%d_%H%M%S"),
            "saved_models",
            "diffuser_experiment_",
        )

    def _train_step(self, train_batch: Dict[str, torch.Tensor]) -> torch.Tensor:
        clean_images = train_batch["images"].to(self.device)
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
        noisy_images = self.noise_scheduler.add_noise(clean_images, noise, timesteps)

        # Get the model prediction
        noise_pred = self.model(noisy_images, timesteps, return_dict=False)[0]

        # Calculate the loss
        return F.mse_loss(noise_pred, noise)

    def _eval_step(self, eval_batch: Dict[str, torch.Tensor]) -> torch.Tensor:
        clean_images = eval_batch["images"].to(self.device)
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
        noisy_images = self.noise_scheduler.add_noise(clean_images, noise, timesteps)
        # Get the model prediction
        noise_pred = self.model(noisy_images, timesteps, return_dict=False)[0]
        # Calculate the loss
        return F.mse_loss(noise_pred, noise)

    def _log_samples(self, epoch: int) -> None:
        x = torch.randn(8, self.channels, self.image_size, self.image_size).to(
            self.device
        )  # Batch of 8
        for i, t in tqdm(enumerate(self.sampling_scheduler.timesteps)):
            model_input = self.sampling_scheduler.scale_model_input(x, t)
            with torch.no_grad():
                noise_pred = self.model(model_input, t)["sample"]
            x = self.sampling_scheduler.step(noise_pred, t, x).prev_sample
        grid = torchvision.utils.make_grid(x, nrow=4)
        im = grid.permute(1, 2, 0).cpu().clip(-1, 1) * 0.5 + 0.5
        im = Image.fromarray(np.array(im * 255).astype(np.uint8))
        wandb.log({"Sample generations": wandb.Image(im)})

    def _save_model(self, epoch: int) -> None:
        DDIMPipeline(self.model, self.sampling_scheduler).save_pretrained(
            self.model_save_name + f"step_{epoch+1}"
        )


def make_diffuser_trainer(
    image_size: int,
    image_mode: str,
    batch_size: int,
    path:str,
    gradient_accumulation_steps: int,
) -> BasicTrainer:
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
    optimizer = torch.optim.AdamW(model.parameters(), lr=0.001)
    lr_scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
        optimizer, 64, eta_min=0.00001
    )
    return DiffuserTrainer(
        model,
        SimpleDataloaderManager(path, image_mode, image_size, batch_size),
        optimizer,
        lr_scheduler,
        gradient_accumulation_steps,
    )


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
    trainer = make_diffuser_trainer(
        image_size, image_mode, batch_size, root, gradient_accumulation_steps
    )
    trainer.train(epochs)
