import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))


from datasets import load_dataset
from diffusers import DDPMScheduler, DDIMScheduler, UNet2DModel
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


def make_data_loader(local_path, image_mode="RGB", image_size=32, batch_size=64):
    dataset = load_dataset("imagefolder", data_dir=local_path, split="train")

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


def train(train_dataloader, model, device, grad_accumulation_steps=1):
    # Set the noise scheduler
    noise_scheduler = DDPMScheduler(
        num_train_timesteps=1000, beta_schedule="squaredcos_cap_v2"
    )

    # Get a scheduler for sampling
    sampling_scheduler = DDIMScheduler.from_config(model)
    sampling_scheduler.set_timesteps(num_inference_steps=50)

    # Training loop
    optimizer = torch.optim.AdamW(model.parameters(), lr=4e-4)

    losses = []
    i = 0
    log_samples_every = 100

    for epoch in range(30):
        for step, batch in enumerate(train_dataloader):
            clean_images = batch["images"].to(device)
            # Sample noise to add to the images
            noise = torch.randn(clean_images.shape).to(clean_images.device)
            bs = clean_images.shape[0]

            # Sample a random timestep for each image
            timesteps = torch.randint(
                0,
                noise_scheduler.num_train_timesteps,
                (bs,),
                device=clean_images.device,
            ).long()

            # Add noise to the clean images according to the noise magnitude at each timestep
            noisy_images = noise_scheduler.add_noise(clean_images, noise, timesteps)

            # Get the model prediction
            noise_pred = model(noisy_images, timesteps, return_dict=False)[0]

            # Calculate the loss
            loss = F.mse_loss(noise_pred, noise)
            loss.backward(loss)
            losses.append(loss.item())
            wandb.log({"loss": loss})

            # Update the model parameters with the optimizer
            # Gradient accumulation:
            i += 1
            if i % grad_accumulation_steps == 0:
                optimizer.step()
                optimizer.zero_grad()

            # Occasionally log samples
            if (step + 1) % log_samples_every == 0:
                x = torch.randn(8, 3, 256, 256).to(device)  # Batch of 8
                for i, t in tqdm(enumerate(sampling_scheduler.timesteps)):
                    model_input = sampling_scheduler.scale_model_input(x, t)
                    with torch.no_grad():
                        noise_pred = model(model_input, t)["sample"]
                    x = sampling_scheduler.step(noise_pred, t, x).prev_sample
                grid = torchvision.utils.make_grid(x, nrow=4)
                im = grid.permute(1, 2, 0).cpu().clip(-1, 1) * 0.5 + 0.5
                im = Image.fromarray(np.array(im * 255).astype(np.uint8))
                wandb.log({"Sample generations": wandb.Image(im)})

        loss_last_epoch = sum(losses[-len(train_dataloader) :]) / len(train_dataloader)
        print(f"Epoch:{epoch+1}, loss: {loss_last_epoch}")
        wandb.log({"epoch_loss": loss_last_epoch})
        # image_pipe.save_pretrained(model_save_name+f'step_{step+1}')


if __name__ == "__main__":
    load_dotenv()
    random.seed(20221213)
    root = os.path.join(os.path.dirname(__file__), "tmp", "dataset")
    if not os.path.exists(root):
        os.makedirs(root)
    cache_blobs(
        root,
        os.environ.get("AZURE_BLOB_CONNECTION_STR"),
        os.environ.get("AZURE_BLOB_CONTAINER_NAME"),
        0.0,
        500,
    )
    wandb.init(project="diffusers experiment", entity="cian-m-hayes")
    dataloader = make_data_loader(root, "L", 128, 8)
    model = make_model(128, 1, "cpu")
    train(dataloader, model, "cpu", 8)
