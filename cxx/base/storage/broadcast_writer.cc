#include "broadcast_writer.h"

namespace base {
namespace storage {

BroadcastWriter::BroadcastWriter(
    std::vector<std::unique_ptr<Writer>> internal_writers)
    : internal_writers_(std::move(internal_writers)) {}

BroadcastWriter::~BroadcastWriter() {}

void BroadcastWriter::Write(uint8_t* data, size_t size) {
  for (auto& w : internal_writers_) {
    if (w) {
      w->Write(data, size);
    }
  }
}

void BroadcastWriter::Close() {
  for (auto& w : internal_writers_) {
    if (w) {
      w->Close();
    }
  }
}

}  // namespace storage
}  // namespace base