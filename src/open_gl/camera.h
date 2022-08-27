#ifndef SRC_OPEN_GL_CAMERA_H_
#define SRC_OPEN_GL_CAMERA_H_

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
 public:
  Camera(glm::vec3&& pos, glm::vec3&& front, glm::vec3&& up);
  ~Camera();
  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  void ProcessInput(GLFWwindow* window, float delta_time);

  void ProcessMouseInput(GLFWwindow* window, double xpos, double ypos);

  glm::mat4 GetViewMatrix();

  glm::mat4 GetProjectionMatrix();

  glm::vec3 GetPosition();

 private:
  glm::vec3 pos_;
  glm::vec3 front_;
  glm::vec3 up_;

  float last_mouse_x_;
  float last_mouse_y_;
  float yaw_ = -90.0f;
  float pitch_ = 0.0f;
  bool first_mouse_ = true;
};

#endif  // SRC_OPEN_GL_CAMERA_H_