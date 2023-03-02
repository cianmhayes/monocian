from flask import Flask, request
import json
import os
import requests
from PIL import Image
from io import BytesIO
from urllib.parse import urlparse

app = Flask(__name__)

def get_output_folder():
    script_path = os.path.dirname(__file__)
    path = os.path.join(script_path, "output")
    if not os.path.exists(path):
        os.makedirs(path)
    return path

def get_metadata_path():
    return os.path.join(get_output_folder(), "metadata.json")

def get_image_folder():
    path = os.path.join(get_output_folder(), "images")
    if not os.path.exists(path):
        os.makedirs(path)
    return path

def persist_metadata(new_metadata):
    metadata_path = get_metadata_path()
    metadata = []
    if os.path.exists(metadata_path):
        with open(metadata_path, "r", encoding="utf-8") as metadata_file:
            metadata = json.load(metadata_file)
    if not any([m["page_url"] == new_metadata["page_url"] for m in metadata if "page_url" in m]):
        metadata.append(new_metadata)
    with open(metadata_path, "w", encoding="utf-8") as metadata_file:
        json.dump(metadata, metadata_file, indent=4)

def download_image(url):
    filename = urlparse(url).path.strip("/").replace("/", "_")
    output_path = os.path.join(get_image_folder(), filename)
    if not os.path.exists(output_path):
        r = requests.get(url)
        i = Image.open(BytesIO(r.content))
        i.save(output_path)

def persist_download(new_download):
    metadata_path = get_metadata_path()
    if os.path.exists(metadata_path):
        metadata = []
        with open(metadata_path, "r", encoding="utf-8") as metadata_file:
            metadata = json.load(metadata_file)
        for i in range(len(metadata)):
            if "page_url" in metadata[i] and metadata[i]["page_url"] == new_download["metadata_url"] :
                if "download_urls" not in metadata[i]:
                    metadata[i]["download_urls"] = []
                if not any([d["download_page_url"] == new_download["download_page_url"] for d in metadata[i]["download_urls"] if "download_page_url" in d]):
                    metadata[i]["download_urls"].append({"download_page_url": new_download["download_page_url"], "download_url": new_download["download_url"]})
                    download_image(new_download["download_url"])
                with open(metadata_path, "w", encoding="utf-8") as metadata_file:
                    json.dump(metadata, metadata_file, indent=4)
                return
            

@app.route('/save_metadata', methods=['POST'])
def save_metadata():
    print(request.method)
    print(request.path)
    print(request.content_type)
    print(request.is_json)
    print(json.dumps(request.get_json()))
    persist_metadata(request.get_json())
    
    return "", 200

@app.route('/save_download', methods=['POST'])
def save_download():
    print(request.method)
    print(request.path)
    print(request.content_type)
    print(request.is_json)
    print(json.dumps(request.get_json()))
    persist_download(request.get_json())
    
    return "", 200
