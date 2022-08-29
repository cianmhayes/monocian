#include "file_writer.h"

FileWriter::FileWriter(const char* filename) {
  output_file_.open(filename, std::ifstream::binary | std::ifstream::out);
}

FileWriter::~FileWriter() {}

void FileWriter::Write(uint8_t* data, size_t size) {
  output_file_.write(reinterpret_cast<char*>(data), size);
}

void FileWriter::Close() {
  output_file_.close();
}