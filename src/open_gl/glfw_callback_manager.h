#ifndef SRC_OPEN_GL_GLFW_CALLBACK_MANAGER_H_
#define SRC_OPEN_GL_GLFW_CALLBACK_MANAGER_H_

#include <GLFW/glfw3.h>
#include <map>
#include <memory>
#include <vector>

class FramebufferSizeCallback {
 public:
  FramebufferSizeCallback() = default;
  virtual ~FramebufferSizeCallback() = default;
  FramebufferSizeCallback(const FramebufferSizeCallback&) = delete;
  FramebufferSizeCallback& operator=(const FramebufferSizeCallback&) = delete;
  virtual void Run(GLFWwindow* window, int width, int height) = 0;
};

class ProcessMouseInputCallback {
 public:
  ProcessMouseInputCallback() = default;
  virtual ~ProcessMouseInputCallback() = default;
  ProcessMouseInputCallback(const ProcessMouseInputCallback&) = delete;
  ProcessMouseInputCallback& operator=(const ProcessMouseInputCallback&) =
      delete;
  virtual void Run(GLFWwindow* window, double x_pos, double y_pos) = 0;
};

class GlfwCallbackManager {
 public:
  GlfwCallbackManager();
  ~GlfwCallbackManager();
  GlfwCallbackManager(const GlfwCallbackManager&) = delete;
  GlfwCallbackManager& operator=(const GlfwCallbackManager&) = delete;

  static GlfwCallbackManager* GetInstance();

  void AddFramebufferSizeCallback(
      GLFWwindow* window,
      std::unique_ptr<FramebufferSizeCallback> callback);

  void AddProcessMouseInputCallback(
      GLFWwindow* window,
      std::unique_ptr<ProcessMouseInputCallback> callback);

  void NotifyFrameBufferSizeCallbacks(GLFWwindow* window,
                                      int width,
                                      int height);

  void NotifyProcessMouseInputCallbacks(GLFWwindow* window,
                                        double x_pos,
                                        double y_pos);

 private:
  std::map<GLFWwindow*, std::vector<std::unique_ptr<FramebufferSizeCallback>>>
      framebuffer_size_callbacks_;

  std::map<GLFWwindow*, std::vector<std::unique_ptr<ProcessMouseInputCallback>>>
      process_mouse_input_callbacks_;
};

#endif  // SRC_OPEN_GL_GLFW_CALLBACK_MANAGER_H_