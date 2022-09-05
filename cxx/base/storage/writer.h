#ifndef BASE_STORAGE_WRITER_H_
#define BASE_STORAGE_WRITER_H_

namespace base {
namespace storage {

class Writer {
 public:
  virtual void Write(uint8_t* data, size_t size) = 0;

  virtual void Close() = 0;
};

}  // namespace storage
}  // namespace base

#endif  // BASE_STORAGE_WRITER_H_