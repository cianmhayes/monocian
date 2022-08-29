#include "broadcast_writer.h"

BroadcastWriter::BroadcastWriter(std::vector<Writer*> internal_writers)
    : internal_writers_(internal_writers) {}

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