#ifndef AV_VIDEO_ENCODING_QUEUE_H
#define AV_VIDEO_ENCODING_QUEUE_H

#include <memory>

#include "base/async_processing_queue.h"
#include "base/storage/writer.h"

namespace av {

class VideoEncoder;

class VideoEncodingQueue
    : public base::AsyncProcessingQueueBase<std::vector<uint8_t>> {
 public:
  VideoEncodingQueue(std::unique_ptr<base::storage::Writer> writer,
                     int fps,
                     int width,
                     int height,
                     int bitrate);
  ~VideoEncodingQueue() override;
  VideoEncodingQueue(const VideoEncodingQueue&) = delete;
  VideoEncodingQueue& operator=(const VideoEncodingQueue&) = delete;

 protected:
  bool Startup() override;

  void Shutdown() override;

  void ProcessItem(std::vector<uint8_t>&& item) override;

  std::unique_ptr<VideoEncoder> encoder_;
};

}  // namespace av

#endif  // AV_VIDEO_ENCODING_QUEUE_H