#ifndef SRC_REASOURCES_LOAD_RESOURCE_H_
#define SRC_REASOURCES_LOAD_RESOURCE_H_

#include <string>

class Resource {
 public:
  Resource(const char* start, const size_t len)
      : resource_data(start), data_len(len) {}

  const char* const& data() const { return resource_data; }
  const size_t& size() const { return data_len; }

  const char* begin() const { return resource_data; }
  const char* end() const { return resource_data + data_len; }

  std::string ToString() { return std::string(data(), size()); }

 private:
  const char* resource_data;
  const size_t data_len;
};

#ifdef __cplusplus

#define DECLARE_RESOURCE(RESOURCE)                \
  extern "C" {                                    \
  extern const char _resource_##RESOURCE[];       \
  extern const size_t _resource_##RESOURCE##_len; \
  }\

#else

#define DECLARE_RESOURCE(RESOURCE)                \
  extern const char _resource_##RESOURCE[];       \
  extern const size_t _resource_##RESOURCE##_len; \

#endif // __cplusplus

#define LOAD_RESOURCE(RESOURCE) \
  Resource(_resource_##RESOURCE, _resource_##RESOURCE##_len);

#endif  // SRC_REASOURCES_LOAD_RESOURCE_H_