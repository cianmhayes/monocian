#include "window.h"

#include <GLFW/glfw3.h>

namespace ogl {

Window::Window(int32_t width, int32_t height, std::string title, bool resizable)
    : width_(width), height_(height) {
  glfwInit();
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);
  glfw_window_ =
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(glfw_window_);
  glfwSetWindowUserPointer(glfw_window_, this);

  glfwSetFramebufferSizeCallback(
      glfw_window_, [](GLFWwindow* w, int width, int height) {
        Window* this_w = (Window*)glfwGetWindowUserPointer(w);
        this_w->NotifyFrameBufferSizeCallbacks(width, height);
      });

  glfwSetCursorPosCallback(
      glfw_window_, [](GLFWwindow* w, double x_position, double y_position) {
        Window* this_w = (Window*)glfwGetWindowUserPointer(w);
        this_w->NotifyMouseMovementCallbacks(x_position, y_position);
      });

  glfwSetKeyCallback(glfw_window_, [](GLFWwindow* w, int key, int scancode,
                                      int action, int mods) {
    Window* this_w = (Window*)glfwGetWindowUserPointer(w);
    if (action == GLFW_PRESS)
      this_w->NotifyKeyPressedCallbacks(key);
    else if (action == GLFW_RELEASE)
      this_w->NotifyKeyReleasedCallbacks(key);
  });

  glfwSetMouseButtonCallback(
      glfw_window_, [](GLFWwindow* w, int button, int action, int mods) {
        Window* this_w = (Window*)glfwGetWindowUserPointer(w);
        if (action == GLFW_PRESS)
          this_w->NotifyMouseButtonPressedCallbacks(button);
        else if (action == GLFW_RELEASE)
          this_w->NotifyMouseButtonReleasedCallbacks(button);
      });

  glfwSetScrollCallback(glfw_window_,
                        [](GLFWwindow* w, double xoffset, double yoffset) {
                          Window* this_w = (Window*)glfwGetWindowUserPointer(w);
                          this_w->NotifyMouseScrollCallbacks(xoffset, yoffset);
                        });
  frame_start_ = glfwGetTime();
  last_frame_duration_ = 0.0f;
};

Window::~Window() {
  glfwDestroyWindow(glfw_window_);
  glfwTerminate();
};

bool Window::IsValid() const {
  return glfw_window_ != 0;
}

bool Window::ShouldClose() const {
  return glfwWindowShouldClose(glfw_window_);
}

void Window::SetShouldClose(bool should_close) {
  glfwSetWindowShouldClose(glfw_window_, should_close);
}

void Window::SwapBuffers() {
  glfwSwapBuffers(glfw_window_);
}

bool Window::IsKeyPressed(int32_t key) {
  return glfwGetKey(glfw_window_, key) == GLFW_PRESS;
}

void Window::SetMouseCursorMode(MouseCursorMode mode) {
  switch (mode) {
    case MouseCursorMode::kHidden:
      glfwSetInputMode(glfw_window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
      break;
    case MouseCursorMode::kDisabled:
      glfwSetInputMode(glfw_window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      break;
    default:
      glfwSetInputMode(glfw_window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      break;
  }
}

void Window::SetTitle(const std::string& new_title) {
  glfwSetWindowTitle(glfw_window_, new_title.c_str());
}

bool Window::FrameStart() {
  if (!IsValid()) {
    return false;
  }
  if (frame_started_) {
    FrameEnd();
  }
  glfwGetWindowSize(glfw_window_, &width_, &height_);
  glClear(GL_COLOR_BUFFER_BIT);

  float last_frame_start = frame_start_;
  frame_start_ = glfwGetTime();
  last_frame_duration_ = frame_start_ - last_frame_start;
  frame_started_ = true;
  return !ShouldClose();
}

void Window::FrameEnd() {
  SwapBuffers();
  glfwPollEvents();
  frame_started_ = false;
}

float Window::GetLastFrameDuration() {
  return last_frame_duration_;
}

float Window::GetTimeSinceFrameStart() {
  return glfwGetTime() - frame_start_;
}

Window::CallbackCookie Window::AddFramebufferSizeCallback(
    Window::FramebufferSizeCallback callback) {
  Window::CallbackCookie c = framebuffer_size_callbacks_.size();
  framebuffer_size_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::RemoveFramebufferSizeCallback(Window::CallbackCookie cookie) {
  framebuffer_size_callbacks_.erase(cookie);
}

void Window::NotifyFrameBufferSizeCallbacks(int32_t width, int32_t height) {
  for (const auto& cb : framebuffer_size_callbacks_) {
    cb.second(this, width, height);
  }
}

Window::CallbackCookie Window::AddMouseMovementCallback(
    Window::MouseMovementCallback callback) {
  Window::CallbackCookie c = mouse_movement_callbacks_.size();
  mouse_movement_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::RemoveMouseMovementCallback(Window::CallbackCookie cookie) {
  mouse_movement_callbacks_.erase(cookie);
}

void Window::NotifyMouseMovementCallbacks(double x_position,
                                          double y_position) {
  for (const auto& cb : mouse_movement_callbacks_) {
    cb.second(this, x_position, y_position);
  }
}

Window::CallbackCookie Window::AddMouseButtonPressedCallback(
    Window::MouseButtonPressedCallback callback) {
  Window::CallbackCookie c = key_pressed_callbacks_.size();
  mouse_button_pressed_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::NotifyMouseButtonPressedCallbacks(int32_t key) {
  for (const auto& cb : mouse_button_pressed_callbacks_) {
    cb.second(this, key);
  }
}

void Window::RemoveMouseButtonPressedCallback(Window::CallbackCookie cookie) {
  mouse_button_pressed_callbacks_.erase(cookie);
}

Window::CallbackCookie Window::AddMouseButtonReleasedCallback(
    Window::MouseButtonReleasedCallback callback) {
  Window::CallbackCookie c = mouse_button_released_callbacks_.size();
  mouse_button_released_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::NotifyMouseButtonReleasedCallbacks(int32_t key) {
  for (const auto& cb : mouse_button_released_callbacks_) {
    cb.second(this, key);
  }
}

void Window::RemoveMouseButtonReleasedCallback(Window::CallbackCookie cookie) {
  mouse_button_released_callbacks_.erase(cookie);
}

Window::CallbackCookie Window::AddMouseScrollCallback(
    Window::MouseScrollCallback callback) {
  Window::CallbackCookie c = mouse_scroll_callbacks_.size();
  mouse_scroll_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::NotifyMouseScrollCallbacks(double x_offset, double y_offset) {
  for (const auto& cb : mouse_scroll_callbacks_) {
    cb.second(this, x_offset, y_offset);
  }
}

void Window::RemoveMouseScrollCallback(Window::CallbackCookie cookie) {
  mouse_scroll_callbacks_.erase(cookie);
}

Window::CallbackCookie Window::AddKeyPressedCallback(
    Window::KeyPressedCallback callback) {
  Window::CallbackCookie c = key_pressed_callbacks_.size();
  key_pressed_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::NotifyKeyPressedCallbacks(int32_t key) {
  for (const auto& cb : key_pressed_callbacks_) {
    cb.second(this, key);
  }
}

void Window::RemoveKeyPressedCallback(Window::CallbackCookie cookie) {
  key_pressed_callbacks_.erase(cookie);
}

Window::CallbackCookie Window::AddKeyReleasedCallback(
    Window::KeyReleasedCallback callback) {
  Window::CallbackCookie c = key_released_callbacks_.size();
  key_released_callbacks_.emplace(c, std::move(callback));
  return c;
}

void Window::NotifyKeyReleasedCallbacks(int32_t key) {
  for (const auto& cb : key_released_callbacks_) {
    cb.second(this, key);
  }
}

void Window::RemoveKeyReleasedCallback(Window::CallbackCookie cookie) {
  key_released_callbacks_.erase(cookie);
}

}  // namespace ogl