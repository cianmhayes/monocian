import torch


def make_stacked_conv_block(in_channels: int, out_channels: int) -> torch.nn.Sequential:
    return torch.nn.Sequential(
        torch.nn.Conv2d(in_channels, out_channels, kernel_size=3, padding=1),
        torch.nn.BatchNorm2d(out_channels),
        torch.nn.ReLU(inplace=True),
        torch.nn.Conv2d(out_channels, out_channels, kernel_size=3, padding=1),
        torch.nn.BatchNorm2d(out_channels),
        torch.nn.ReLU(inplace=True),
    )


class UNetDownBlock(torch.nn.Module):
    def __init__(
        self, in_channels: int, out_channels: int, pool=True, dropout=True
    ) -> None:
        super().__init__()
        self.pool = torch.nn.MaxPool2d(2) if pool else None
        self.conv = make_stacked_conv_block(in_channels, out_channels)
        self.dropout = torch.nn.Dropout() if dropout else None

    def forward(self, input: torch.Tensor) -> torch.Tensor:
        output = self.conv(self.pool(input) if self.pool else input)
        return self.dropout(output) if self.dropout else output


class UNetUpBlock(torch.nn.Module):
    def __init__(self, in_channels: int, out_channels: int, dropout=True) -> None:
        super().__init__()
        self.conv_transpose = torch.nn.ConvTranspose2d(
            in_channels, in_channels // 2, kernel_size=2, stride=2
        )
        self.conv = make_stacked_conv_block(in_channels, out_channels)
        self.dropout = torch.nn.Dropout()

    def forward(
        self, input: torch.Tensor, skip_connection: torch.Tensor
    ) -> torch.Tensor:
        transpose_input = self.conv_transpose(input)
        concatenated = torch.cat([transpose_input, skip_connection], dim=1)
        output = self.conv(concatenated)
        return self.dropout(output) if self.dropout else output
