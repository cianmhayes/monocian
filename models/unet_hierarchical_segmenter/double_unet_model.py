import torch
from unet_blocks import UNetDownBlock, UNetUpBlock


class Classifier(torch.nn.Module):
    def __init__(self, in_channels: int, out_channels: int) -> None:
        super().__init__()
        self.classifier = torch.nn.Conv2d(in_channels, out_channels, kernel_size=1)
        self.softmax = torch.nn.Softmax(1)

    def forward(self, input: torch.Tensor) -> torch.Tensor:
        return self.softmax(self.classifier(input))

class Reconstructor(torch.nn.Module):
    def __init__(self, in_channels: int, out_channels: int) -> None:
        super().__init__()
        self.reconstructor = torch.nn.Conv2d(in_channels, out_channels, kernel_size=1)
        self.activation = torch.nn.ReLU(True)

    def forward(self, input: torch.Tensor) -> torch.Tensor:
        return self.activation(self.reconstructor(input))

class DoubleUNet(torch.nn.Module):
    def __init__(
        self, in_channels, n_classes, embeddings=[64, 128, 256, 512, 1024]
    ) -> None:
        super().__init__()
        self.down1 = UNetDownBlock(in_channels, embeddings[0], pool=False)
        self.down2 = UNetDownBlock(embeddings[0], embeddings[1])
        self.down3 = UNetDownBlock(embeddings[1], embeddings[2])
        self.down4 = UNetDownBlock(embeddings[2], embeddings[3])
        self.bottle_neck = UNetDownBlock(embeddings[3], embeddings[4])

        self.reconstruction_up1 = UNetUpBlock(embeddings[4], embeddings[3])
        self.reconstruction_up2 = UNetUpBlock(embeddings[3], embeddings[2])
        self.reconstruction_up3 = UNetUpBlock(embeddings[2], embeddings[1])
        self.reconstruction_up4 = UNetUpBlock(embeddings[1], embeddings[0])
        self.reconstructor = Reconstructor(embeddings[0], in_channels)

        self.classification_up1 = UNetUpBlock(embeddings[4], embeddings[3])
        self.classification_up2 = UNetUpBlock(embeddings[3], embeddings[2])
        self.classification_up3 = UNetUpBlock(embeddings[2], embeddings[1])
        self.classification_up4 = UNetUpBlock(embeddings[1], embeddings[0])
        self.classifier = Classifier(embeddings[0], n_classes)

    def forward(self, x):
        x1 = self.down1(x)
        x2 = self.down2(x1)
        x3 = self.down3(x2)
        x4 = self.down4(x3)
        x5 = self.bottle_neck(x4)

        reconstruction_x6 = self.reconstruction_up1(x5, x4)
        reconstruction_x7 = self.reconstruction_up2(reconstruction_x6, x3)
        reconstruction_x8 = self.reconstruction_up3(reconstruction_x7, x2)
        reconstruction_x9 = self.reconstruction_up4(reconstruction_x8, x1)

        classification_x6 = self.classification_up1(x5, x4)
        classification_x7 = self.classification_up2(classification_x6, x3)
        classification_x8 = self.classification_up3(classification_x7, x2)
        classification_x9 = self.classification_up4(classification_x8, x1)

        return self.reconstructor(reconstruction_x9), self.classifier(classification_x9)


if __name__ == "__main__":
    sample = torch.ones(2, 3, 512, 512)
    model = DoubleUNet(3, 5)
    reconstructed, classified = model(sample)
    print(sample.shape)
    print(reconstructed.shape)
    print(classified.shape)
