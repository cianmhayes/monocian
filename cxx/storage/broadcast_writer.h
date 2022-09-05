#ifndef STORAGE_BROADCAST_WRITER_H_
#define STORAGE_BROADCAST_WRITER_H_

#include <memory>
#include <vector>
#include "writer.h"

class BroadcastWriter : public Writer {
 public:
  BroadcastWriter(std::vector<std::unique_ptr<Writer>> internal_writers);
  ~BroadcastWriter();
  BroadcastWriter(const BroadcastWriter&) = delete;
  BroadcastWriter& operator=(const BroadcastWriter&) = delete;

  virtual void Write(uint8_t* data, size_t size) override;

  virtual void Close() override;

 private:
  std::vector<std::unique_ptr<Writer>> internal_writers_;
};

#endif  // STORAGE_BROADCAST_WRITER_H_