#ifndef BASE_STORAGE_FILE_WRITER_H_
#define BASE_STORAGE_FILE_WRITER_H_

#include <fstream>
#include "writer.h"

namespace base {
namespace storage {

class FileWriter : public Writer {
 public:
  FileWriter(const char* filename);
  ~FileWriter();
  FileWriter(const FileWriter&) = delete;
  FileWriter& operator=(const FileWriter&) = delete;

  void Write(uint8_t* data, size_t size) override;

  void Close() override;

 private:
  std::ofstream output_file_;
};

}  // namespace storage
}  // namespace base

#endif  // BASE_STORAGE_FILE_WRITER_H_