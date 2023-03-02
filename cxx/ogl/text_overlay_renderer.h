#ifndef CXX_OGL_TEXT_OVERLAY_RENDERER_H
#define CXX_OGL_TEXT_OVERLAY_RENDERER_H

#include <string>

#include "Window.h"

namespace ogl {

class TextOverlayRenderer {
 public:
  TextOverlayRenderer(Window* window, float x, float y, const std::string& content);
  ~TextOverlayRenderer() = default;
  TextOverlayRenderer(const TextOverlayRenderer&) = delete;
  TextOverlayRenderer& operator=(const TextOverlayRenderer&) = delete;

  void SetLocation(float x, float y);

  void SetContent(const std::string& new_content);

  void Render();

 private:
  Window* window_;
  float x_;
  float y_;
  std::string content_;
};

}  // namespace ogl

#endif  // CXX_OGL_TEXT_OVERLAY_RENDERER_H