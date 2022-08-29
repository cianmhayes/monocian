#ifndef TOOLS_FERRY_WRITER_H
#define TOOLS_FERRY_WRITER_H

class Writer {
 public:
  virtual void Write(uint8_t* data, size_t size) = 0;

  virtual void Close() = 0;
};

#endif  // TOOLS_FERRY_WRITER_H