#include "video_encoding_queue.h"
#include "video_encoder.h"

VideoEncodingQueue::VideoEncodingQueue(VideoEncoder* encoder)
    : encoder_(encoder) {}

VideoEncodingQueue::~VideoEncodingQueue() {}

bool VideoEncodingQueue::Startup() {
  if (encoder_) {
    encoder_->Init();
    return true;
  }
  return false;
}

void VideoEncodingQueue::Shutdown() {
  if (encoder_) {
    encoder_->Stop();
  }
}

void VideoEncodingQueue::ProcessItem(std::vector<uint8_t>&& item) {
  encoder_->AddFrame(item);
}