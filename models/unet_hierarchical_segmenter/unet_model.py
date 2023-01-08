import torch
from unet_blocks import UNetDownBlock, UNetUpBlock

class Classifier(torch.nn.Module):
    def __init__(self, in_channels: int, out_channels: int) -> None:
        super().__init__()
        self.classifier = torch.nn.Conv2d(in_channels, out_channels, kernel_size=1)
        self.softmax = torch.nn.Softmax(1)

    def forward(self, input: torch.Tensor) -> torch.Tensor:
        return self.softmax(self.classifier(input))


class UNet(torch.nn.Module):
    def __init__(
        self, in_channels, n_classes, embeddings=[64, 128, 256, 512, 1024]
    ) -> None:
        super().__init__()
        self.down1 = UNetDownBlock(in_channels, embeddings[0], pool=False)
        self.down2 = UNetDownBlock(embeddings[0], embeddings[1])
        self.down3 = UNetDownBlock(embeddings[1], embeddings[2])
        self.down4 = UNetDownBlock(embeddings[2], embeddings[3])
        self.bottle_neck = UNetDownBlock(embeddings[3], embeddings[4])

        self.up1 = UNetUpBlock(embeddings[4], embeddings[3])
        self.up2 = UNetUpBlock(embeddings[3], embeddings[2])
        self.up3 = UNetUpBlock(embeddings[2], embeddings[1])
        self.up4 = UNetUpBlock(embeddings[1], embeddings[0])
        self.classifier = Classifier(embeddings[0], n_classes)

    def forward(self, x):
        x1 = self.down1(x)
        x2 = self.down2(x1)
        x3 = self.down3(x2)
        x4 = self.down4(x3)
        x5 = self.bottle_neck(x4)

        x6 = self.up1(x5, x4)
        x7 = self.up2(x6, x3)
        x8 = self.up3(x7, x2)
        x9 = self.up4(x8, x1)
        return self.classifier(x9)


if __name__ == "__main__":
    sample = torch.ones(2, 3, 512, 512)
    model = UNet(3, 5)
    output = model(sample)
    print(sample.shape)
    print(output.shape)
