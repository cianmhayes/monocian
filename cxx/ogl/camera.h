#ifndef CXX_OGL_CAMERA_H_
#define CXX_OGL_CAMERA_H_

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace ogl {

class Camera {
 public:
  Camera(glm::vec3&& pos);
  Camera(glm::vec3&& pos, glm::vec3&& front, glm::vec3&& up);
  ~Camera();
  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  void ProcessDirectionKeys(bool forward,
                            bool back,
                            bool left,
                            bool right,
                            float camera_speed);

  void ProcessMouseInput(double xpos, double ypos);

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

}  // namespace ogl

#endif  // CXX_OGL_CAMERA_H_