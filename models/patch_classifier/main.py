import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import base64
import numpy as np
import pyarrow as pa
import pyarrow.parquet as pq
import random
import torch
from arrow_util import *
from cluster_trainer import ClusterTrainer
from common.datasets.azure_container_image_dataset import cache_blobs
from dotenv import load_dotenv
from image_patch_featurizer import PatchFeaturizer
from typing import Tuple, List, Dict
from PIL import Image
from torchvision.transforms import ToTensor
from tqdm import tqdm

from io import BytesIO
from jinja2 import Environment, FileSystemLoader, select_autoescape


def load_image(path: str, patch_size: int, mode: str) -> torch.Tensor:
    i = Image.open(path)
    i = i.convert(mode)
    width, height = i.size
    width_remainder = width % patch_size
    height_remainder = height % patch_size
    i = i.crop((0, 0, width - width_remainder, height - height_remainder))
    return ToTensor()(i)


def load_image_as_patches(path: str, patch_size: int, mode: str) -> List[Image.Image]:
    i = Image.open(path)
    i = i.convert(mode)
    width, height = i.size
    patches = []
    for x in range(0, width, patch_size):
        for y in range(0, height, patch_size):
            patches.append(i.crop((x, y, x + patch_size, y + patch_size)))
    return patches


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


def get_target_device() -> torch.device:
    if torch.cuda.is_available():
        print("Using cuda")
        return torch.device("cuda")
    return torch.device("cpu")


def image_to_base64(image: Image.Image) -> str:
    buffered = BytesIO()
    image.save(buffered, format="JPEG")
    img_str = base64.b64encode(buffered.getvalue()).decode("utf-8")
    return "data:image/jpeg;base64," + img_str


def build_patch_dataset(
    train_data_path,
    validation_data_path,
    output_path,
    nclusters=[8, 16, 24, 32, 64, 128],
):
    clustering_models = [ClusterTrainer(nc) for nc in nclusters]
    device = get_target_device()
    pf = PatchFeaturizer()
    pf.to(device)
    pf.eval()
    print("Fitting models")
    for img_path in tqdm(list_all_files(train_data_path)):
        img_tensor = torch.unsqueeze(load_image(img_path, 32, "RGB"), 0)
        img_tensor = img_tensor.to(device)
        features = pf(img_tensor)
        for cm in clustering_models:
            cm.fit(features.cpu())
    print()
    print("Preparing validation data")
    validation_data = {}
    for img_path in tqdm(list_all_files(validation_data_path)):
        patches = load_image_as_patches(img_path, 32, "RGB")
        encoded_patches = list([image_to_base64(p) for p in patches])
        t = torch.stack([ToTensor()(p) for p in patches])
        t = t.to(device)
        features = pf(t)
        features = features.cpu()
        for i in range(len(encoded_patches)):
            validation_data[encoded_patches[i]] = features[i]
    for cm in clustering_models:
        print()
        print(f"Evaluating model with {cm.n_clusters()} clusters")
        generate_clustering_model_report(validation_data, output_path, cm, pf, device)


def generate_clustering_model_report(
    validation_set: Dict[str, torch.Tensor],
    output_path: str,
    cm: ClusterTrainer,
    pf: PatchFeaturizer,
    device: torch.device,
):
    clustered_output = list([[] for i in range(cm.n_clusters())])
    max_samples = 100
    for (p, t) in tqdm(validation_set.items()):
        cluster = cm.predict(torch.unsqueeze(t, 0))[0]
        assert cluster < len(clustered_output)
        if len(clustered_output[cluster]) < max_samples:
            clustered_output[cluster].append(p)
        # Stop if we've maxed out all clusters
        if all(
            [
                len(clustered_output[i]) >= max_samples
                for i in range(len(clustered_output))
            ]
        ):
            break
    env = Environment(
        loader=FileSystemLoader(os.path.join(os.path.dirname(__file__), "templates")),
        autoescape=select_autoescape(),
    )
    template = env.get_template("clustering_examples.html.jinja")
    with open(
        os.path.join(output_path, f"minibatch_kmeans_k_{cm.n_clusters()}.html"),
        "w",
        encoding="utf-8",
    ) as report_file:
        report_file.write(template.render({"clusters": clustered_output}))
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
        0.2,
        0,
        10000,
    )
    with torch.no_grad():
        build_patch_dataset(
            os.path.join(root, "train"), os.path.join(root, "validation"), output
        )
