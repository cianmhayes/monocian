#ifndef SRC_OPEN_GL_WINDOW_H_
#define SRC_OPEN_GL_WINDOW_H_

#include <functional>
#include <map>

class Window;
struct GLFWwindow;

using CallbackCookie = size_t;

using FramebufferSizeCallback =
    std::function<void(Window* window, int32_t width, int32_t height)>;
using MouseMovementCallback =
    std::function<void(Window* window, double x_position, double y_position)>;

using MouseButtonPressedCallback =
    std::function<void(Window* window, int32_t button)>;
using MouseButtonReleasedCallback =
    std::function<void(Window* window, int32_t button)>;
using MouseScrollCallback =
    std::function<void(Window* window, double x_offset, double y_offset)>;

using KeyPressedCallback = std::function<void(Window* window, int32_t key)>;
using KeyReleasedCallback = std::function<void(Window* window, int32_t key)>;

class Window {
 public:
  Window(int width, int height, std::string title, bool resizable);
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  operator bool() { return IsValid() && !ShouldClose(); }

  bool IsValid() const;
  bool ShouldClose() const;
  void SetShouldClose(bool should_close);
  void SwapBuffers();
  bool IsKeyPressed(int32_t key);

  CallbackCookie AddFramebufferSizeCallback(FramebufferSizeCallback callback);
  void NotifyFrameBufferSizeCallbacks(int32_t width, int32_t height);
  void RemoveFramebufferSizeCallback(CallbackCookie cookie);

  CallbackCookie AddMouseMovementCallback(MouseMovementCallback callback);
  void NotifyMouseMovementCallbacks(double x_position, double y_position);
  void RemoveMouseMovementCallback(CallbackCookie cookie);

  CallbackCookie AddMouseButtonPressedCallback(
      MouseButtonPressedCallback callback);
  void NotifyMouseButtonPressedCallbacks(int32_t key);
  void RemoveMouseButtonPressedCallback(CallbackCookie cookie);

  CallbackCookie AddMouseButtonReleasedCallback(
      MouseButtonReleasedCallback callback);
  void NotifyMouseButtonReleasedCallbacks(int32_t key);
  void RemoveMouseButtonReleasedCallback(CallbackCookie cookie);

  CallbackCookie AddMouseScrollCallback(MouseScrollCallback callback);
  void NotifyMouseScrollCallbacks(double x_offset, double y_offset);
  void RemoveMouseScrollCallback(CallbackCookie cookie);

  CallbackCookie AddKeyPressedCallback(KeyPressedCallback callback);
  void NotifyKeyPressedCallbacks(int32_t key);
  void RemoveKeyPressedCallback(CallbackCookie cookie);

  CallbackCookie AddKeyReleasedCallback(KeyReleasedCallback callback);
  void NotifyKeyReleasedCallbacks(int32_t key);
  void RemoveKeyReleasedCallback(CallbackCookie cookie);

 private:
  std::map<CallbackCookie, FramebufferSizeCallback> framebuffer_size_callbacks_;
  std::map<CallbackCookie, MouseMovementCallback> mouse_movement_callbacks_;
  std::map<CallbackCookie, MouseButtonPressedCallback>
      mouse_button_pressed_callbacks_;
  std::map<CallbackCookie, MouseButtonReleasedCallback>
      mouse_button_released_callbacks_;
  std::map<CallbackCookie, MouseScrollCallback> mouse_scroll_callbacks_;
  std::map<CallbackCookie, KeyPressedCallback> key_pressed_callbacks_;
  std::map<CallbackCookie, KeyReleasedCallback> key_released_callbacks_;
  GLFWwindow* glfw_window_;
};

#endif  // SRC_OPEN_GL_WINDOW_H_