
#include "video_encoder.h"

#include <functional>
#include <iostream>

#include <glog/logging.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "storage/writer.h"

#define RNDTO2(X) ( ( (X) & 0xFFFFFFFE )
#define RNDTO32(X) (((X) % 32) ? (((X) + 32) & 0xFFFFFFE0) : (X))

static int WriteCallback(void* opaque, uint8_t* buf, int buf_size) {
  Writer* writer = reinterpret_cast<Writer*>(opaque);
  writer->Write((uint8_t*)buf, buf_size);
  return buf_size;
}

void log_callback(void* ptr, int level, const char* fmt, va_list vargs) {
  printf("\n%s", fmt);
}

void enable_av_logging() {
  av_log_set_level(AV_LOG_VERBOSE);
  av_log_set_callback(log_callback);
}

VideoEncoder::VideoEncoder(Writer* writer,
                           int fps,
                           int width,
                           int height,
                           int bitrate)
    : writer_(writer),
      fps_(fps),
      width_(width),
      height_(height),
      bitrate_(bitrate),
      pts_(0),
      initialized_(false),
      stopped_(false) {}

VideoEncoder::~VideoEncoder() {
  if (pkt_to_write_) {
    av_packet_free(&pkt_to_write_);
  }
  if (output_ctx_) {
    avformat_free_context(output_ctx_);
  }
  if (av_ctx_) {
    avcodec_free_context(&av_ctx_);
  }
  if (rgb_to_yuv_ctx_) {
    sws_freeContext(rgb_to_yuv_ctx_);
  }
}

bool VideoEncoder::Init() {
  // Initialize codec
  codec_ = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264);
  if (!codec_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avcodec_find_encoder failed to allocate codec";
    return false;
  }
  av_ctx_ = avcodec_alloc_context3(codec_);
  if (!av_ctx_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avcodec_alloc_context3 failed to allocate context";
    return false;
  }

  av_ctx_->bit_rate = bitrate_;
  av_ctx_->width = width_;
  av_ctx_->height = height_;
  av_ctx_->time_base = {1, fps_};
  av_ctx_->framerate = {fps_, 1};
  av_ctx_->gop_size = 10;
  av_ctx_->max_b_frames = 1;
  av_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;

  int ret = avcodec_open2(av_ctx_, codec_, NULL);
  if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avcodec_open2 returned " << ret;
    return false;
  }

  // Initialize output context
  const size_t buffer_size = 500 * 1024;
  uint8_t* ctx_buffer = (uint8_t*)(av_malloc(buffer_size));
  if (ctx_buffer == NULL) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "av_malloc failed to allocate buffer";
    return false;
  }

  int io_mode = 1;  // 0 for reading, 1 for writing
  avio_output_ctx_ = avio_alloc_context(ctx_buffer, buffer_size, io_mode,
                                        reinterpret_cast<void*>(writer_), NULL,
                                        &WriteCallback, NULL);

  ret = avformat_alloc_output_context2(&output_ctx_, NULL, "asf",
                                       "placeholder_filename");
  if (ret < 0 || !output_ctx_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avformat_alloc_output_context2 returned " << ret
               << "or failed to allocate context";
    return false;
  }
  output_ctx_->pb = avio_output_ctx_;
  output_ctx_->flags |= AVFMT_FLAG_CUSTOM_IO | AVFMT_NOFILE;

  // Create output stream
  out_stream_ = avformat_new_stream(output_ctx_, NULL);
  if (!out_stream_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avformat_new_stream failed to allocate new stream";
    return false;
  }
  out_stream_->time_base = av_ctx_->time_base;
  out_stream_->avg_frame_rate = av_ctx_->framerate;

  ret = avcodec_parameters_from_context(out_stream_->codecpar, av_ctx_);
  if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avcodec_parameters_from_context returned " << ret;
    return false;
  }

  // set stream codec tag to 0, for libav to detect automatically
  out_stream_->codecpar->codec_tag = 0;

  ret = avformat_write_header(output_ctx_, NULL);
  if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avformat_write_header returned " << ret;
    return false;
  }

  // Create the frame transform context to create YUV frames from RGB
  rgb_to_yuv_ctx_ = sws_getContext(
      width_, height_, AV_PIX_FMT_RGB24, width_, height_, AV_PIX_FMT_YUV420P,
      SWS_LANCZOS | SWS_ACCURATE_RND, NULL, NULL, NULL);
  if (!rgb_to_yuv_ctx_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "sws_getContext failed to create conversion context.";
    return false;
  }

  pkt_to_write_ = av_packet_alloc();
  if (!pkt_to_write_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "Failed toallocate output packet";
    return false;
  }

  frame_ = av_frame_alloc();
  if (!frame_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "av_frame_alloc failed to allocate frame";
    return false;
  }

  initialized_ = true;
  return true;
}

bool VideoEncoder::AddFrame(const std::vector<uint8_t>& frame_data) {
  if (!initialized_ || stopped_) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "Not ready";
    return false;
  }

  av_frame_unref(frame_);

  int ret = 0;
  frame_->format = av_ctx_->pix_fmt;
  frame_->width = av_ctx_->width;
  frame_->height = av_ctx_->height;
  frame_->pts = pts_;
  pts_ += av_rescale_q(1, av_ctx_->time_base, out_stream_->time_base);

  ret = av_frame_get_buffer(frame_, 0);
  if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "av_frame_get_buffer returned " << ret;
    return false;
  }
  ret = av_frame_make_writable(frame_);
  if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "av_frame_make_writable returned " << ret;
    return false;
  }
  int src_stride[] = {width_ * 3};
  const uint8_t* src_planes[] = {frame_data.data()};
  ret = sws_scale(rgb_to_yuv_ctx_, (const uint8_t* const*)src_planes,
                  src_stride, 0, height_, frame_->data, frame_->linesize);
  if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "sws_scale returned " << ret;
    return false;
  }
  ret = avcodec_send_frame(av_ctx_, frame_);
  if (ret == AVERROR(EAGAIN)) {
    if (!DrainPackets()) {
      LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
                 << "failed to send packets";
      return false;
    }
    ret = avcodec_send_frame(av_ctx_, frame_);
  } else if (ret < 0) {
    LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
               << "avcodec_send_frame returned " << ret;
    return false;
  }

  return true;
}

void VideoEncoder::Stop() {
  stopped_ = true;
  avcodec_send_frame(av_ctx_, NULL);
  DrainPackets();
  writer_->Close();
}

bool VideoEncoder::DrainPackets() {
  if (!initialized_) {
    return true;
  }

  int ret = 0;
  std::vector<uint8_t> buffer = {};
  while (ret >= 0) {
    av_packet_unref(pkt_to_write_);
    ret = avcodec_receive_packet(av_ctx_, pkt_to_write_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      continue;
    else if (ret < 0) {
      LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
                 << "avcodec_receive_packet returned " << ret;
      return false;
    }
    ret = av_interleaved_write_frame(output_ctx_, pkt_to_write_);
    if (ret < 0) {
      LOG(ERROR) << "VideoEncoder::" << __FUNCTION__ << "\t"
                 << "av_interleaved_write_frame returned " << ret;
      return false;
    }
  }
  return true;
}