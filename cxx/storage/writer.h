#ifndef STORAGE_WRITER_H_
#define STORAGE_WRITER_H_

class Writer {
 public:
  virtual void Write(uint8_t* data, size_t size) = 0;

  virtual void Close() = 0;
};

#endif  // STORAGE_WRITER_H_