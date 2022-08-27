#include "glfw_callback_manager.h"

void global_framebuffer_size_callback(GLFWwindow* window,
                                      int width,
                                      int height) {
  GlfwCallbackManager::GetInstance()->NotifyFrameBufferSizeCallbacks(
      window, width, height);
}

void global_process_mpuse_input_callback(GLFWwindow* window,
                                         double x_pos,
                                         double y_pos) {
  GlfwCallbackManager::GetInstance()->NotifyProcessMouseInputCallbacks(
      window, x_pos, y_pos);
}

GlfwCallbackManager::GlfwCallbackManager() = default;
GlfwCallbackManager::~GlfwCallbackManager() = default;

// static
GlfwCallbackManager* GlfwCallbackManager::GetInstance() {
  static std::unique_ptr<GlfwCallbackManager> instance =
      std::make_unique<GlfwCallbackManager>();
  return instance.get();
}

void GlfwCallbackManager::AddFramebufferSizeCallback(
    GLFWwindow* window,
    std::unique_ptr<FramebufferSizeCallback> callback) {
  auto& it = framebuffer_size_callbacks_.find(window);
  if (it == framebuffer_size_callbacks_.end()) {
    glfwSetFramebufferSizeCallback(window, global_framebuffer_size_callback);
    std::vector<std::unique_ptr<FramebufferSizeCallback>> v = {};
    v.push_back(std::move(callback));
    framebuffer_size_callbacks_.emplace(std::make_pair(window, std::move(v)));
  } else {
    it->second.push_back(std::move(callback));
  }
}

void GlfwCallbackManager::AddProcessMouseInputCallback(
    GLFWwindow* window,
    std::unique_ptr<ProcessMouseInputCallback> callback) {
  auto& it = process_mouse_input_callbacks_.find(window);
  if (it == process_mouse_input_callbacks_.end()) {
    glfwSetCursorPosCallback(window, global_process_mpuse_input_callback);
    std::vector<std::unique_ptr<ProcessMouseInputCallback>> v = {};
    v.push_back(std::move(callback));
    process_mouse_input_callbacks_.emplace(
        std::make_pair(window, std::move(v)));
  } else {
    it->second.push_back(std::move(callback));
  }
}

void GlfwCallbackManager::NotifyFrameBufferSizeCallbacks(GLFWwindow* window,
                                                         int width,
                                                         int height) {
  auto& it = framebuffer_size_callbacks_.find(window);
  for (const auto& cb : it->second) {
    cb->Run(window, width, height);
  }
}

void GlfwCallbackManager::NotifyProcessMouseInputCallbacks(GLFWwindow* window,
                                                           double x_pos,
                                                           double y_pos) {
  auto& it = process_mouse_input_callbacks_.find(window);
  for (const auto& cb : it->second) {
    cb->Run(window, x_pos, y_pos);
  }
}