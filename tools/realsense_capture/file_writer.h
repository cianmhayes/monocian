#ifndef TOOLS_FERRY_FILE_WRITER_H
#define TOOLS_FERRY_FILE_WRITER_H

#include <fstream>

#include "writer.h"

class FileWriter : public Writer {
 public:
  FileWriter(const char* filename);
  ~FileWriter();
  FileWriter(const FileWriter&) = delete;
  FileWriter& operator=(const FileWriter&) = delete;

  virtual void Write(uint8_t* data, size_t size) override;

  virtual void Close() override;

 private:
  std::ofstream output_file_;
};

#endif  // TOOLS_FERRY_FILE_WRITER_H