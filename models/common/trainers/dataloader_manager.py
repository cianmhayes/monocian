from torch.utils.data import DataLoader
from abc import ABC, abstractmethod

class DataloaderManager(ABC):
    def __init__(self) -> None:
        self.epoch = 0

    def notify_epoch_start(self, epoch:int) -> None:
        self.epoch = epoch

    @abstractmethod
    def get_training_dataloader(self) -> DataLoader:
        pass

    @abstractmethod
    def get_testing_dataloader(self) -> DataLoader:
        pass

    @abstractmethod
    def get_validation_dataloader(self) -> DataLoader:
        pass
