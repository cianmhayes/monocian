import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import torch
import numpy as np
import pyarrow as pa
import pyarrow.parquet as pq
from arrow_util import *
from cluster_trainer import ClusterTrainer
from image_patch_featurizer import PatchFeaturizer
from typing import Tuple, List, NamedTuple
from common.datasets.azure_container_image_dataset import cache_blobs
from dotenv import load_dotenv
import random
from PIL import Image
from torchvision.transforms import ToTensor
from tqdm import tqdm


def load_image(path: str, patch_size: int, mode: str) -> torch.Tensor:
    i = Image.open(path)
    i = i.convert(mode)
    width, height = i.size
    width_remainder = width % patch_size
    height_remainder = height % patch_size
    i = i.crop((0, 0, width - width_remainder, height - height_remainder))
    return ToTensor()(i)


def build_patch_dataset(output_path):
    pf = PatchFeaturizer()
    ct = ClusterTrainer()
    r = torch.rand([100, 512])
    ct.fit(r)
    predictions = ct.predict(r)
    n = float_tensor_to_arrow_array(r)
    table = pa.Table.from_arrays([n, predictions], names=[pf.feature_name(), "cluster"])
    pq.write_table(table, output_path, compression=None)


def read_patch_dataset(path) -> Tuple[torch.Tensor, torch.Tensor]:
    table = pq.read_table(path)
    features = arrow_bytes_array_to_float_tensor(table.column(0), 512)
    labels = arrow_int_array_to_long_tensor(table.column(1))
    return (features, labels)


def list_all_files(root) -> List[str]:
    output = []
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            output.append(os.path.join(dirpath, fn))
    return output


def get_target_device():
    if torch.cuda.is_available():
        print("Using cuda")
        return torch.device("cuda")
    return torch.device("cpu")


def build_patch_dataset(data_path, output_path, nclusters = [8, 16]):
    clustering_models = [ClusterTrainer(nc) for nc in nclusters]
    pf = PatchFeaturizer()
    pf.to(get_target_device())
    pf.eval()
    for img_path in tqdm(list_all_files(data_path)):
        img_tensor = torch.unsqueeze(load_image(img_path, 32, "RGB"), 0)
        features = pf(img_tensor)
        for cm in clustering_models:
            cm.fit(features)
    for cm in clustering_models:
        cm.pickle(os.path.join(output_path, f"minibatch_kmeans_k{cm.n_clusters()}.pickle"))


if __name__ == "__main__":
    load_dotenv()
    random.seed(20230108)
    root = os.path.join(os.path.dirname(__file__), "tmp", "clustering_dataset")
    output = os.path.join(os.path.dirname(__file__), "tmp", "clustering_models")
    if not os.path.exists(root):
        os.makedirs(root)
    if not os.path.exists(output):
        os.makedirs(output)
    cache_blobs(
        root,
        os.environ.get("AZURE_BLOB_CONNECTION_STR"),
        os.environ.get("AZURE_BLOB_CONTAINER_NAME"),
        0,
        0,
        10000,
    )
    with torch.no_grad():
        build_patch_dataset(
            os.path.join(root, "train"), output
        )
