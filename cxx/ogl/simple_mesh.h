#ifndef CXX_OGL_SIMPLE_MESH_H_
#define CXX_OGL_SIMPLE_MESH_H_

#include <map>
#include <string>
#include <vector>

namespace ogl {

class SimpleMesh {
 public:
  SimpleMesh();
  ~SimpleMesh();
  SimpleMesh(const SimpleMesh&) = delete;
  SimpleMesh& operator=(const SimpleMesh&) = delete;

  void SetTexture(const std::string& texture_key,
                  int32_t width,
                  int32_t height,
                  const void* data);
  void SetVertices(std::vector<float> vertex_data,
                   std::vector<size_t> attrib_sizes);
  void Draw();

  uint32_t GetTextureHandle(const std::string& texture_key) const;

 private:
  uint32_t vao_handle_;
  uint32_t vbo_handle_;
  size_t vertex_count_;
  std::map<std::string, uint32_t> texture_handles_;
};
}  // namespace ogl

#endif  // CXX_OGL_SIMPLE_MESH_H_