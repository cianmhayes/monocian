#ifndef SRC_OPEN_GL_FULL_SCREEN_VIDEO_H_
#define SRC_OPEN_GL_FULL_SCREEN_VIDEO_H_

#include "Window.h"

enum class FrameFormat { RGB_8, RGBA_8, Y_8, Y_10BPACK };

template <typename TFrame>
const void* GetFrameData(const TFrame& frame);

template <typename TFrame>
int GetFrameWidth(const TFrame& frame);

template <typename TFrame>
int GetFrameHeight(const TFrame& frame);

struct Rectangle {
  float x;
  float y;
  float width;
  float height;
};

Rectangle AdjustBounds(const Rectangle original_bounds,
                       float target_width,
                       float target_height);

class FullScreenVideo {
 public:
  FullScreenVideo(Window* window);
  ~FullScreenVideo();
  FullScreenVideo(const FullScreenVideo&) = delete;
  FullScreenVideo& operator=(const FullScreenVideo&) = delete;

  template <typename TFrame>
  void RenderFrame(const TFrame& frame, FrameFormat format, float alpha = 1.f) {
    int32_t frame_width = GetFrameWidth(frame);
    int32_t frame_height = GetFrameHeight(frame);
    SetFrameContent(frame_width, frame_height, GetFrameData(frame), format);
    ShowFrame(AdjustBounds({0.0f, 0.0f, static_cast<float>(window_->Width()), static_cast<float>(window_->Height())},
                           frame_width, frame_height),
              alpha);
  };

 private:
  void SetFrameContent(int32_t width,
                       int32_t height,
                       const void* data,
                       FrameFormat format);
  void ShowFrame(Rectangle region, float alpha);

  Window* window_;
  uint32_t texture_handle_;
};

#endif  // SRC_OPEN_GL_FULL_SCREEN_VIDEO_H_