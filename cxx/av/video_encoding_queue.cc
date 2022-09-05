#include "video_encoding_queue.h"
#include "video_encoder.h"

namespace av {

VideoEncodingQueue::VideoEncodingQueue(
    std::unique_ptr<base::storage::Writer> writer,
    int fps,
    int width,
    int height,
    int bitrate)
    : encoder_(std::make_unique<VideoEncoder>(std::move(writer),
                                              fps,
                                              width,
                                              height,
                                              bitrate)) {}

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

}  // namespace av