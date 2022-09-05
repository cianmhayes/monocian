#include "buffered_blob_writer.h"

#include <glog/logging.h>
#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>
#include <iostream>
#include <iterator>
#include <string>

using namespace Azure::Storage;
using namespace Azure::Storage::Blobs;
using namespace Azure::Core::IO;

namespace az {

BufferedBlobWriter::BufferedBlobWriter(const std::string& connection_string,
                                       const std::string& container,
                                       const std::string& blob_name,
                                       int max_chunk_size_bytes)
    : connection_string_(connection_string),
      container_(container),
      blob_name_(blob_name),
      max_chunk_size_bytes_(max_chunk_size_bytes) {}

BufferedBlobWriter::~BufferedBlobWriter() {}

void BufferedBlobWriter::Write(uint8_t* data, size_t size) {
  std::copy_n(data, size, std::back_inserter(pending_bytes_));
  if (pending_bytes_.size() > max_chunk_size_bytes_) {
    SendBytes();
  }
}

void BufferedBlobWriter::Close() {
  SendBytes();
}

void BufferedBlobWriter::SendBytes() {
  if (pending_bytes_.size() > 0) {
    auto blob_client_ = AppendBlobClient::CreateFromConnectionString(
        connection_string_, container_, blob_name_);
    auto blobCreateResponse = blob_client_.CreateIfNotExists();
    if (blobCreateResponse.RawResponse) {
      LOG(INFO) << "BufferedBlobWriter::" << __FUNCTION__ << "\t"
                << "Response to creation check on " << blob_name_ << " : "
                << static_cast<int>(
                       blobCreateResponse.RawResponse.get()->GetStatusCode());
    } else {
      LOG(INFO) << "BufferedBlobWriter::" << __FUNCTION__ << "\t"
                << "Missing response from AppendBlobClient::CreateIfNotExists";
    }
    auto appendResponse =
        blob_client_.AppendBlock(MemoryBodyStream(pending_bytes_));
    if (appendResponse.RawResponse) {
      LOG(INFO) << "BufferedBlobWriter::" << __FUNCTION__ << "\t"
                << "Response to append block on " << blob_name_ << " : "
                << static_cast<int>(
                       appendResponse.RawResponse.get()->GetStatusCode());
    } else {
      LOG(INFO) << "BufferedBlobWriter::" << __FUNCTION__ << "\t"
                << "Missing response from AppendBlobClient::AppendBlock";
    }
    pending_bytes_.clear();
  }
}

}  // namespace az