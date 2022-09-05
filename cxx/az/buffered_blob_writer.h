#ifndef AZ_BUFFERED_BLOB_WRITER_H_
#define AZ_BUFFERED_BLOB_WRITER_H_

#include <string>
#include <vector>
#include "base/storage/writer.h"

namespace az {

class BufferedBlobWriter : public base::storage::Writer {
 public:
  BufferedBlobWriter(const std::string& connection_string,
                     const std::string& container,
                     const std::string& blob_name,
                     int max_chunk_size_bytes = 1024 * 1024);
  ~BufferedBlobWriter();
  BufferedBlobWriter(const BufferedBlobWriter&) = delete;
  BufferedBlobWriter& operator=(const BufferedBlobWriter&) = delete;

  void Write(uint8_t* data, size_t size) override;

  void Close() override;

 private:
  void SendBytes();

  std::string connection_string_;
  std::string container_;
  std::string blob_name_;
  int max_chunk_size_bytes_;
  std::vector<uint8_t> pending_bytes_;
};

}  // namespace az

#endif  // AZ_BUFFERED_BLOB_WRITER_H_