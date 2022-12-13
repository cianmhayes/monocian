#include "simple_mesh.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace ogl {

SimpleMesh::SimpleMesh() = default;
SimpleMesh::~SimpleMesh() = default;

void SimpleMesh::SetTexture(const std::string& texture_key,
                            int32_t width,
                            int32_t height,
                            const void* data) {
  if (!data)
    return;

  uint32_t texture_handle = GetTextureHandle(texture_key);
  if (texture_handle == 0) {
    glGenTextures(1, &texture_handle);
    texture_handles_[texture_key] = texture_handle;
  }

  glBindTexture(GL_TEXTURE_2D, texture_handle);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //glGenerateMipmap(GL_TEXTURE_2D);
  // glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void SimpleMesh::SetVertices(std::vector<float> vertex_data,
                             std::vector<size_t> attrib_sizes) {
  if (vao_handle_ == 0)
    glGenVertexArrays(1, &vao_handle_);
  if (vbo_handle_ == 0)
    glGenBuffers(1, &vbo_handle_);

  glBindVertexArray(vao_handle_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);

  glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(),
               GL_STATIC_DRAW);

  size_t stride = 0;
  for (int32_t a : attrib_sizes)
    stride += a;

  size_t offset = 0;
  for (size_t i = 0; i < attrib_sizes.size(); i++) {
    glVertexAttribPointer(i, attrib_sizes[i], GL_FLOAT, GL_FALSE,
                          stride * sizeof(float), (void*)(offset * sizeof(float)));
    glEnableVertexAttribArray(i);
    offset += attrib_sizes[i];
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  vertex_count_ = vertex_data.size() / stride;
}

void SimpleMesh::Draw() {
  glBindVertexArray(vao_handle_);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
}

uint32_t SimpleMesh::GetTextureHandle(const std::string& texture_key) const {
  auto texture_handle_it = texture_handles_.find(texture_key);
  if (texture_handle_it != texture_handles_.end())
    return texture_handle_it->second;
  return 0;
}

}  // namespace ogl
