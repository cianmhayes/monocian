#include "shader.h"

#include <glm/gtc/type_ptr.hpp>

Shader::Shader(std::string&& vertex_shader_source,
               std::string&& fragment_shader_source) {
  is_valid_ = false;
  uint32_t vertex_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  const char* vertex_shader_c_str = vertex_shader_source.c_str();
  glShaderSource(vertex_shader, 1, &vertex_shader_c_str, nullptr);
  glCompileShader(vertex_shader);
  int success;
  char status[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, status);
    error_ = std::string(status);
    return;
  }

  uint32_t fragment_shader;
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* fragment_shader_c_str = fragment_shader_source.c_str();
  glShaderSource(fragment_shader, 1, &fragment_shader_c_str, nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, status);
    error_ = std::string(status);
    return;
  }

  program_id_ = glCreateProgram();
  glAttachShader(program_id_, vertex_shader);
  glAttachShader(program_id_, fragment_shader);
  glLinkProgram(program_id_);
  glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program_id_, 512, NULL, status);
    error_ = std::string(status);
    return;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  is_valid_ = true;
}

Shader::~Shader() = default;

void Shader::use() {
  if (is_valid_)
    glUseProgram(program_id_);
}

void Shader::set_mat4f(const char* name, const glm::mat4& matrix) {
  uint32_t location = glGetUniformLocation(program_id_, name);
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::set_vec3f(const char* name, float v0, float v1, float v2) {
    uint32_t location = glGetUniformLocation(program_id_, name);
    glUniform3f(location, v0, v1, v2);
}

void Shader::set_vec4f(const char* name, float v0, float v1, float v2, float v3) {
    uint32_t location = glGetUniformLocation(program_id_, name);
    glUniform4f(location, v0, v1, v2, v3);
}

void Shader::set_int32(const char* name, uint32_t v0) {
    uint32_t location = glGetUniformLocation(program_id_, name);
    glUniform1i(location, v0);
}
