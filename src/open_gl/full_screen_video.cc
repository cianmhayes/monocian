#include "full_screen_video.h"

#include <GLFW/glfw3.h>

Rectangle AdjustBounds(const Rectangle original_bounds,
                       float target_width,
                       float target_height) {
  float ratio = target_width / target_height;
  Rectangle new_bounds = {0.0f, 0.0f, original_bounds.height * ratio,
                          original_bounds.height};
  if (new_bounds.width > original_bounds.width) {
    float scale = original_bounds.width / new_bounds.width;
    new_bounds.width *= scale;
    new_bounds.height *= scale;
  }
  new_bounds.x = (original_bounds.width - new_bounds.width) / 2;
  new_bounds.y = (original_bounds.height - new_bounds.height) / 2;
  return new_bounds;
}

FullScreenVideo::FullScreenVideo(Window* window) : window_(window){};

FullScreenVideo::~FullScreenVideo() = default;

void FullScreenVideo::SetFrameContent(int32_t frame_width,
                                      int32_t frame_height,
                                      const void* data,
                                      FrameFormat format) {
  if (!data)
    return;
  if (!texture_handle_)
    glGenTextures(1, &texture_handle_);

  glBindTexture(GL_TEXTURE_2D, texture_handle_);

  switch (format) {
    case FrameFormat::RGB_8:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, data);
      break;
    case FrameFormat::RGBA_8:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, data);
      break;
    case FrameFormat::Y_8:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0,
                   GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
      break;
    case FrameFormat::Y_10BPACK:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame_width, frame_height, 0,
                   GL_LUMINANCE, GL_UNSIGNED_SHORT, data);
      break;
    default:
      return;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void FullScreenVideo::ShowFrame(Rectangle region, float alpha) {
  if (!texture_handle_)
    return;

  glPushMatrix();
  // set viewport
  glViewport(region.x, region.y, region.width, region.height);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glOrtho(0, region.width, region.height, 0, -1, +1);

  // configure texture
  glBindTexture(GL_TEXTURE_2D, texture_handle_);
  glColor4f(1.0f, 1.0f, 1.0f, alpha);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);

  glTexCoord2f(0, 0);
  glVertex2f(0, 0);

  glTexCoord2f(0, 1);
  glVertex2f(0, region.height);

  glTexCoord2f(1, 1);
  glVertex2f(region.width, region.height);

  glTexCoord2f(1, 0);
  glVertex2f(region.width, 0);

  glEnd();
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  glPopMatrix();
}