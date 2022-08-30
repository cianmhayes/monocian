#ifndef SRC_OPEN_GL_SHADER_H_
#define SRC_OPEN_GL_SHADER_H_

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {

 public:
  Shader(std::string&& vertex_shader_source, std::string&& fragment_shader_source);
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  void use();
  uint32_t program_id() const { return program_id_; };
  bool is_valid() const { return is_valid_; };
  const std::string& error() const { return error_; };

  void set_mat4f(const char* name, const glm::mat4& matrix);
  void set_vec3f(const char* name, float v0, float v1, float v2);
  void set_vec4f(const char* name, float v0, float v1, float v2, float v3);
  void set_int32(const char* name, int32_t v0);
  void set_float(const char* name, float v0);

 private:
  uint32_t program_id_;
  bool is_valid_;
  std::string error_;
};


#endif SRC_OPEN_GL_SHADER_H_