import torch
from torchvision.models import vgg16, VGG16_Weights
from torchvision.models.feature_extraction import create_feature_extractor


def rearrange(x: torch.Tensor) -> torch.Tensor:
    feature_count = x.shape[1]
    # batches, features, height, width => features, batches, height, width
    x = torch.transpose(x, 0, 1)
    # features, batches, height, width => features, patches
    x = torch.reshape(x, [feature_count, -1])
    # features, patches => patches, features
    x = torch.transpose(x, 0, 1)
    return x


class PatchFeaturizer(torch.nn.Module):
    def __init__(self, input_patch_size=32) -> None:
        super().__init__()
        self.feature_output = "features.30"
        self.output_feature_name = "vgg16_feature30"
        m = vgg16(weights=VGG16_Weights.DEFAULT)
        self.extractor = create_feature_extractor(m, return_nodes=[self.feature_output])
        if input_patch_size != 32:
            self.upsample = torch.nn.Upsample(size=(32,32), mode="bilinear")

    def feature_name(self):
        return self.output_feature_name

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        assert len(x.shape) == 4
        if x.shape[1] == 1:
            x = torch.tile(x, (1, 3, 1, 1))
            #x = torch.cat((x,x,x), 1)
        if self.upsample:
            x = self.upsample(x)
        assert x.shape[2] % 32 == 0
        assert x.shape[3] % 32 == 0
        assert x.shape[1] == 3
        f = self.extractor(x)[self.feature_output]
        return rearrange(f)
