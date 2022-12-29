import torch
import wandb
from abc import ABC, abstractmethod
from .dataloader_manager import DataloaderManager
from torch.utils.data import DataLoader
from torch.optim import Optimizer
from torch.optim.lr_scheduler import _LRScheduler
from tqdm import tqdm
from typing import Dict


class BasicTrainer(ABC):
    def __init__(
        self,
        model: torch.nn.Module,
        dataloader_manager: DataloaderManager,
        optimizer: Optimizer,
        lr_scheduler: _LRScheduler = None,
        gradient_accumulation_steps: int = 1,
    ) -> None:
        self.device = torch.device("cpu")
        if torch.cuda.is_available():
            self.device = torch.device("cuda")
            print("Using cuda")
        model.to(self.device)
        self.model = model
        self.dataloader_manager = dataloader_manager
        self.gradient_accumulation_steps = gradient_accumulation_steps

        self.optimizer = optimizer
        self.lr_scheduler = lr_scheduler

    def _step_optimizer(self) -> None:
        self.optimizer.step()
        self.optimizer.zero_grad()
        if self.lr_scheduler:
            self.lr_scheduler.step()

    def _train_phase(self, dataloader: DataLoader) -> float:
        self.model.train()
        losses = []
        pending_optimizer_step = False
        for step, batch in tqdm(enumerate(dataloader)):
            loss = self._train_step(batch)
            loss.backward(loss)
            losses.append(loss.item())
            pending_optimizer_step = True
            metrics = {"training_loss": loss}
            if self.lr_scheduler is not None:
                metrics["lr"] = float(self.lr_scheduler.get_last_lr()[0])
            wandb.log(metrics)

            # Update the model parameters with the optimizer
            # Gradient accumulation:
            if (step + 1) % self.gradient_accumulation_steps == 0:
                pending_optimizer_step = False
                self._step_optimizer()

        if pending_optimizer_step:
            self._step_optimizer()
        return sum(losses) / len(dataloader)

    def _eval_phase(self, dataloader: DataLoader) -> float:
        with torch.no_grad():
            self.model.eval()
            losses = []
            for _, batch in tqdm(enumerate(dataloader)):
                loss = self._eval_step(batch)
                losses.append(loss.item())
            return sum(losses) / len(dataloader)

    @abstractmethod
    def _train_step(self, train_batch: Dict[str, torch.Tensor]) -> torch.Tensor:
        pass

    @abstractmethod
    def _eval_step(self, eval_batch: Dict[str, torch.Tensor]) -> torch.Tensor:
        pass

    @abstractmethod
    def _log_samples(self, epoch: int) -> None:
        pass

    @abstractmethod
    def _save_model(self, epoch: int) -> None:
        pass

    def train(self, epochs):
        lowest_train_loss = None
        lowest_validation_loss = None
        for epoch in range(epochs):
            print("============================================================")
            print(f" Epoch {epoch+1}")
            print("============================================================")
            self.dataloader_manager.notify_epoch_start(epoch)
            should_save_model = False
            metrics = {}
            epoch_train_loss = self._train_phase(
                self.dataloader_manager.get_training_dataloader()
            )
            metrics["Epoch-end Train Loss"] = epoch_train_loss
            print(f"Training loss: {epoch_train_loss:0.4f}")
            print()
            if lowest_train_loss is None or epoch_train_loss < lowest_train_loss:
                should_save_model = True
            validation_data_loader = self.dataloader_manager.get_validation_dataloader()
            if validation_data_loader:
                validation_loss = self._eval_phase(validation_data_loader)
                metrics["Epoch-end Validation Loss"] = validation_loss
                print(f"Validation loss: {validation_loss:0.4f}")
                print()
                if (
                    lowest_validation_loss is None
                    or validation_loss < lowest_validation_loss
                ):
                    should_save_model = True
            test_data_loader = self.dataloader_manager.get_testing_dataloader()
            if test_data_loader:
                test_loss = self._eval_phase(test_data_loader)
                print(f"Test loss: {test_loss:0.4f}")
                print()
                metrics["Epoch-end Test Loss"] = test_loss
            print("Logging")
            wandb.log(metrics)
            self._log_samples(epoch)
            if should_save_model:
                self._save_model(epoch)
