#include "text_overlay_renderer.h"

#include <GLFW/glfw3.h>
#include <stb_easy_font.h>
#include <vector>

namespace ogl {

TextOverlayRenderer::TextOverlayRenderer(Window* window,
                                         float x,
                                         float y,
                                         const std::string& content)
    : window_(window), x_(x), y_(y), content_(content){};

void TextOverlayRenderer::SetLocation(float x, float y) {
  this->x_ = x;
  this->y_ = y;
}

void TextOverlayRenderer::SetContent(const std::string& new_content) {
  this->content_ = new_content;
}

void TextOverlayRenderer::Render() {
  int x = this->x_ * window_->GetWidth();
  int y = this->y_ * window_->GetHeight();
  std::vector<char> buffer;
  buffer.resize(60000);  // ~300 chars
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 16, &(buffer[0]));
  glDrawArrays(
      GL_QUADS, 0,
      4 * stb_easy_font_print((float)x, (float)(y - 7),
                              (char*)this->content_.c_str(), nullptr,
                              &(buffer[0]), int(sizeof(char) * buffer.size())));
  glDisableClientState(GL_VERTEX_ARRAY);
}

}  // namespace ogl