#ifndef CXX_AV_FRAME_RATE_TRACKER_H
#define CXX_AV_FRAME_RATE_TRACKER_H

#include <chrono>
#include <deque>

namespace av {

class FrameRateTracker {
 public:
  FrameRateTracker(uint32_t max_frame_history);
  ~FrameRateTracker() = default;
  FrameRateTracker(const FrameRateTracker&) = delete;
  FrameRateTracker& operator=(const FrameRateTracker&) = delete;

  int32_t get_fps() const;

  void notify_frame_start();

 private:
  uint32_t max_frame_history_;
  std::chrono::high_resolution_clock::time_point last_frame_start_;
  std::deque<double> frame_durations_;
};

}  // namespace av

#endif  // CXX_AV_FRAME_RATE_TRACKER_H