#ifndef TOOLS_FERRY_VIDEO_ENCODING_QUEUE_H
#define TOOLS_FERRY_VIDEO_ENCODING_QUEUE_H

#include "base/async_processing_queue.h"

class VideoEncoder;

class VideoEncodingQueue : public base::AsyncProcessingQueueBase<std::vector<uint8_t>> {
 public:
  VideoEncodingQueue(VideoEncoder* encoder);
  ~VideoEncodingQueue() override;
  VideoEncodingQueue(const VideoEncodingQueue&) = delete;
  VideoEncodingQueue& operator=(const VideoEncodingQueue&) = delete;

 protected:
  bool Startup() override;

  void Shutdown() override;

  void ProcessItem(std::vector<uint8_t>&& item) override;

  VideoEncoder* encoder_;
};

#endif  // TOOLS_FERRY_VIDEO_ENCODING_QUEUE_H