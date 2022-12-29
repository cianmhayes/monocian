import os
from typing import Sequence, Optional
from azure.storage.blob import ContainerClient, BlobClient
import random
import math
import multiprocessing as mp
from functools import partial
from typing import Tuple, List


def list_blobs(
    connection_string: str,
    container_name: str,
    limit: Optional[int],
    name_starts_with: Optional[str],
    bad_blobs:List[str]
) -> Sequence[str]:
    count = 0
    cc = ContainerClient.from_connection_string(connection_string, container_name)
    for blob in cc.list_blobs(name_starts_with=name_starts_with):
        if blob in bad_blobs:
            continue
        if limit is not None and count >= limit:
            break
        if blob and blob.name:
            count += 1
            yield blob.name


def download_blob(
    connection_string: str, container_name: str, download_task:Tuple[str, str]
):
    blob_name, target_path = download_task
    cc = ContainerClient.from_connection_string(connection_string, container_name)
    bc = cc.get_blob_client(blob_name)
    with open(target_path, "wb") as local_blob:
        download_stream = bc.download_blob()
        local_blob.write(download_stream.readall())


def cache_blobs(
    local_folder_path: str,
    connection_string: str,
    container_name: str,
    validation_ratio: float,
    test_ratio: float,
    blob_limit: Optional[int] = None,
    name_starts_with: Optional[str] = None,
    bad_blobs:List[str] = []
) -> None:
    blobs = list(
        list_blobs(connection_string, container_name, blob_limit, name_starts_with, bad_blobs)
    )
    random.shuffle(blobs)
    validation_count = math.ceil(len(blobs) * validation_ratio)
    validation_root = os.path.join(local_folder_path, "validation")
    if not os.path.exists(validation_root):
        os.makedirs(validation_root)
    test_count = math.ceil(len(blobs) * test_ratio)
    test_root = os.path.join(local_folder_path, "test")
    if not os.path.exists(test_root):
        os.makedirs(test_root)
    train_root = os.path.join(local_folder_path, "train")
    if not os.path.exists(train_root):
        os.makedirs(train_root)
    download_tasks = []
    for i in range(len(blobs)):
        local_blob_name = blobs[i].replace("/", "_"). replace("\\", "_")
        folder = train_root
        if i < validation_count:
            folder = validation_root
        elif i < validation_count + test_count:
            folder = test_root
        target_path = os.path.join(folder, local_blob_name)
        if os.path.exists(target_path):
            continue
        download_tasks.append((blobs[i], target_path))
    with mp.Pool(processes=os.cpu_count() or 2) as pool:
        pool.map(partial(download_blob, connection_string, container_name), download_tasks)
