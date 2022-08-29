#ifndef TOOLS_FERRY_VIDEO_ENCODER_H
#define TOOLS_FERRY_VIDEO_ENCODER_H

#include <vector>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVIOContext;
struct AVPacket;
struct AVStream;
struct SwsContext;
class Writer;

void enable_av_logging();

class VideoEncoder {
 public:
  VideoEncoder(Writer* writer, int fps, int width, int height, int bitrate);
  ~VideoEncoder();
  VideoEncoder(const VideoEncoder&) = delete;
  VideoEncoder& operator=(const VideoEncoder&) = delete;

  bool Init();

  bool AddFrame(const std::vector<uint8_t>& frame_data);

  void Stop();

 private:
  bool DrainPackets();

  Writer* writer_;
  int fps_;
  int width_;
  int height_;
  int bitrate_;
  int64_t pts_;
  bool initialized_;
  bool stopped_;

  AVIOContext* avio_output_ctx_;
  AVFormatContext* output_ctx_;
  AVCodec* codec_;
  AVCodecContext* av_ctx_;
  AVStream* out_stream_;
  SwsContext* rgb_to_yuv_ctx_;
  AVPacket* pkt_to_write_;
  AVFrame* frame_;
};

#endif  // TOOLS_FERRY_VIDEO_ENCODER_H