import os
from typing import Sequence, Optional
from azure.storage.blob import ContainerClient, BlobClient
import random
import math
import hashlib

def hash_str(input:str) -> str:
    h = hashlib.new("sha256")
    h.update(input.encode())
    return h.hexdigest()

def list_blobs(connection_string:str, container_name:str, limit:Optional[int]) -> Sequence[str]:
    count = 0
    cc = ContainerClient.from_connection_string(connection_string, container_name)
    for blob in cc.list_blobs():
        if limit is not None and count >= limit :
            break
        if blob and blob.name:
            count += 1
            yield blob.name

def cache_blobs(local_folder_path:str, connection_string:str, container_name:str, test_ratio:float, blob_limit:Optional[int]) -> None:
    blobs = list(list_blobs(connection_string, container_name, blob_limit))
    random.shuffle(blobs)
    cc = ContainerClient.from_connection_string(connection_string, container_name)
    test_count = math.ceil(len(blobs) * test_ratio)
    test_root = os.path.join(local_folder_path, "test")
    if not os.path.exists(test_root):
        os.makedirs(test_root)
    train_root = os.path.join(local_folder_path, "train")
    if not os.path.exists(train_root):
        os.makedirs(train_root)
    for i in range(len(blobs)):
        extension = blobs[i].split(".")[-1]
        blob_name_hash = hash_str(blobs[i])
        target_path = os.path.join(test_root if i < test_count else train_root, blob_name_hash + "." + extension)
        if os.path.exists(target_path):
            continue
        bc = cc.get_blob_client(blobs[i])
        with open(target_path, "wb") as local_blob:
            download_stream = bc.download_blob()
            local_blob.write(download_stream.readall())

