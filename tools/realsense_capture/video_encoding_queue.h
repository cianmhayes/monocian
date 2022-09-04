#ifndef TOOLS_FERRY_VIDEO_ENCODING_QUEUE_H
#define TOOLS_FERRY_VIDEO_ENCODING_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <list>
#include <thread>
#include <vector>

class VideoEncoder;

class VideoEncodingQueue {
 public:
  VideoEncodingQueue(VideoEncoder* encoder);
  ~VideoEncodingQueue();
  VideoEncodingQueue(const VideoEncodingQueue&) = delete;
  VideoEncodingQueue& operator=(const VideoEncodingQueue&) = delete;

  void Start();

  void AddFrame(std::vector<uint8_t> frame);

  void Stop();

 private:
  void ProcessingThread();

  VideoEncoder* encoder_;
  bool running_;
  std::list<std::vector<uint8_t>> q_;
  mutable std::mutex m_;
  std::condition_variable cv_;
  std::thread processing_thread_;
};

#endif  // TOOLS_FERRY_VIDEO_ENCODING_QUEUE_H