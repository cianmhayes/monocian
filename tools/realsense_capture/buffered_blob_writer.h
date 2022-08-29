#ifndef TOOLS_FERRY_BUFFERED_BLOB_WRITER_H
#define TOOLS_FERRY_BUFFERED_BLOB_WRITER_H

#include <string>
#include <vector>

#include "writer.h"

class BufferedBlobWriter : public Writer {
 public:
  BufferedBlobWriter(const std::string& connection_string,
                     const std::string& container,
                     const std::string& blob_name,
                     int max_chunk_size_bytes = 1024 * 1024);
  ~BufferedBlobWriter();
  BufferedBlobWriter(const BufferedBlobWriter&) = delete;
  BufferedBlobWriter& operator=(const BufferedBlobWriter&) = delete;

  virtual void Write(uint8_t* data, size_t size) override;

  void Write(const std::vector<uint8_t>& chunk);

  virtual void Close() override;

 private:

  void SendBytes();

  std::string connection_string_;
  std::string container_;
  std::string blob_name_;
  int max_chunk_size_bytes_;
  std::vector<uint8_t> pending_bytes_;
};

#endif // TOOLS_FERRY_BUFFERED_BLOB_WRITER_H