import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
sys.path.append(
    os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "third_party", "mae")
    )
)


import random
import torch
import wandb
from common.datasets.azure_container_image_dataset import cache_blobs
from common.trainers.basic_trainer import BasicTrainer
from common.trainers.dataloader_manager import DataloaderManager
from datasets import load_dataset
from dotenv import load_dotenv
from functools import partial
from models_mae import mae_vit_base_patch16_dec512d8b, MaskedAutoencoderViT
from torch.optim import Optimizer
from torch.optim.lr_scheduler import _LRScheduler
from torch.utils.data import DataLoader
from torchvision import transforms
from typing import Dict, NamedTuple


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
                transforms.CenterCrop(image_size),
                transforms.ToTensor()
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
        return self.test_dataloader

    def get_training_dataloader(self) -> DataLoader:
        return self.train_dataloader

    def get_validation_dataloader(self) -> DataLoader:
        return self.validation_dataloader


class MaeTrainer(BasicTrainer):
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

    def _train_step(self, train_batch: Dict[str, torch.Tensor]) -> torch.Tensor:
        loss, _, _ = self.model(train_batch["images"].to(self.device))
        return loss

    def _eval_step(self, eval_batch: Dict[str, torch.Tensor]) -> torch.Tensor:
        loss, _, _ = self.model(eval_batch["images"].to(self.device))
        return loss

    def _log_samples(self, epoch: int) -> None:
        return super()._log_samples(epoch)

    def _save_model(self, epoch: int) -> None:
        return super()._save_model(epoch)


class LearningRateHyperParameters(NamedTuple):
    initial_lr: float
    min_lr: float


class MaeHyperParameters(NamedTuple):
    image_size: int
    image_mode: str
    batch_size: int
    gradient_accumulation_steps: int
    encoder_dim: int
    encoer_depth: int
    encoder_heads: int
    decoder_dim: int
    decoder_depth: int
    decoder_heads: int
    lr: LearningRateHyperParameters


def make_trainer(hps: MaeHyperParameters, path: str) -> BasicTrainer:
    model = MaskedAutoencoderViT(
        img_size=hps.image_size,
        patch_size=16,
        embed_dim=hps.encoder_dim,
        depth=hps.encoer_depth,
        num_heads=hps.encoder_heads,
        decoder_embed_dim=hps.decoder_dim,
        decoder_depth=hps.decoder_depth,
        decoder_num_heads=hps.decoder_heads,
        mlp_ratio=4,
        norm_layer=partial(torch.nn.LayerNorm, eps=1e-6),
    )
    optimizer = torch.optim.AdamW(model.parameters(), lr=hps.lr.initial_lr)
    data_loader = SimpleDataloaderManager(
        path, hps.image_mode, hps.image_size, hps.batch_size
    )
    return MaeTrainer(
        model, data_loader, optimizer, None, hps.gradient_accumulation_steps
    )


if __name__ == "__main__":
    load_dotenv()
    random.seed(20221228)
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "tmp", "aa_dataset"))
    if not os.path.exists(root):
        os.makedirs(root)
    cache_blobs(
        root,
        os.environ.get("AZURE_BLOB_CONNECTION_STR"),
        os.environ.get("AZURE_BLOB_CONTAINER_NAME"),
        0.1,
        0.1,
        None,
        "unprocessed_1000px",
        []
    )
    wandb.init(project="Anna Atkins MAE prototype", entity="cian-m-hayes")
    hps = MaeHyperParameters(
        image_size=992,
        image_mode="RGB",
        batch_size=4,
        gradient_accumulation_steps=16,
        encoder_dim=768,
        encoer_depth=12,
        encoder_heads=12,
        decoder_dim=512,
        decoder_depth=8,
        decoder_heads=16,
        lr=LearningRateHyperParameters(0.0004, 0.00001),
    )
    trainer = make_trainer(hps, root)
    trainer.train(100)
