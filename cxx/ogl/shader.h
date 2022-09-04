#ifndef CXX_OGL_SHADER_H_
#define CXX_OGL_SHADER_H_

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace ogl {

class Shader {
 public:
  Shader(std::string&& vertex_shader_source,
         std::string&& fragment_shader_source);
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  void Use();
  uint32_t GetProgramId() const { return program_id_; };
  bool IsValid() const { return is_valid_; };
  const std::string& GetInitializationError() const { return error_; };

  void SetMat4f(const char* name, const glm::mat4& matrix);
  void SetVec3f(const char* name, float v0, float v1, float v2);
  void SetVec4f(const char* name, float v0, float v1, float v2, float v3);
  void SetInt32(const char* name, int32_t v0);
  void SetFloat(const char* name, float v0);

 private:
  uint32_t program_id_;
  bool is_valid_;
  std::string error_;
};

} // namespace ogl

#endif  // CXX_OGL_SHADER_H_