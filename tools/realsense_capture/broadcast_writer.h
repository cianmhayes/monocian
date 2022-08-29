#ifndef TOOLS_FERRY_BROADCAST_WRITER_H
#define TOOLS_FERRY_BROADCAST_WRITER_H

#include <vector>
#include "writer.h"

class BroadcastWriter : public Writer {
 public:
  BroadcastWriter(std::vector<Writer*> internal_writers);
  ~BroadcastWriter();
  BroadcastWriter(const BroadcastWriter&) = delete;
  BroadcastWriter& operator=(const BroadcastWriter&) = delete;

  virtual void Write(uint8_t* data, size_t size) override;

  virtual void Close() override;

 private:
  std::vector<Writer*> internal_writers_;
};

#endif  // TOOLS_FERRY_BROADCAST_WRITER_H