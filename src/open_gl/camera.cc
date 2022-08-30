#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3&& pos)
    : pos_(pos),
      front_(glm::vec3(0.0f, 0.0f, -1.0f)),
      up_(glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(glm::vec3&& pos, glm::vec3&& front, glm::vec3&& up)
    : pos_(pos), front_(front), up_(up){};
Camera::~Camera() = default;

void Camera::ProcessDirectionKeys(bool forward,
                                  bool back,
                                  bool left,
                                  bool right,
                                  float camera_speed) {
  if (forward && !back) {
    pos_ += camera_speed * front_;
  }
  if (back && !forward) {
    pos_ -= camera_speed * front_;
  }
  if (left && !right) {
    pos_ -= glm::normalize(glm::cross(front_, up_)) * camera_speed;
  }
  if (right && !left) {
    pos_ += glm::normalize(glm::cross(front_, up_)) * camera_speed;
  }
}

void Camera::ProcessMouseInput(double xpos, double ypos) {
  if (first_mouse_) {
    last_mouse_x_ = xpos;
    last_mouse_y_ = ypos;
    first_mouse_ = false;
  }
  float x_delta = xpos - last_mouse_x_;
  float y_delta =
      last_mouse_y_ - ypos;  // reverse it because y origin is at the top
  last_mouse_x_ = xpos;
  last_mouse_y_ = ypos;
  const float sensitivity = 0.1f;
  x_delta *= sensitivity;
  y_delta *= sensitivity;
  yaw_ += x_delta;
  pitch_ += y_delta;
  if (pitch_ >= 89.0f)
    pitch_ = 89.0f;
  if (pitch_ <= -89.0f)
    pitch_ = -89.0f;
  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  direction.y = sin(glm::radians(pitch_));
  direction.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_ = glm::normalize(direction);
}

glm::mat4 Camera::GetViewMatrix() {
  return glm::lookAt(pos_, pos_ + front_, up_);
}

glm::mat4 Camera::GetProjectionMatrix() {
  return glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

glm::vec3 Camera::GetPosition() {
  return pos_;
}
