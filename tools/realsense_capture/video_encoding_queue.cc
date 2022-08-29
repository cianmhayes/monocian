#include "video_encoding_queue.h"
#include "video_encoder.h"

VideoEncodingQueue::VideoEncodingQueue(VideoEncoder* encoder)
    : encoder_(encoder), running_(false) {}

VideoEncodingQueue::~VideoEncodingQueue() {}

void VideoEncodingQueue::Start() {
  if (!running_) {
    encoder_->Init();
    running_ = true;
    processing_thread_ = std::thread([this] { ProcessingThread(); });
  }
}

void VideoEncodingQueue::Stop() {
  running_ = false;
  cv_.notify_one();
  if (processing_thread_.joinable()) {
    processing_thread_.join();
  }
  encoder_->Stop();
}

void VideoEncodingQueue::AddFrame(std::vector<uint8_t> frame) {
  {
    std::unique_lock<decltype(m_)> lock(m_);
    q_.push(std::move(frame));
  }
  cv_.notify_one();
}

void VideoEncodingQueue::ProcessingThread() {
  while (running_) {
    std::unique_lock<decltype(m_)> lock(m_);
    cv_.wait(lock, [this] { return !q_.empty() || !running_; });
    if (!q_.empty()) {
      encoder_->AddFrame(q_.front());
      q_.pop();
    }
  }
}