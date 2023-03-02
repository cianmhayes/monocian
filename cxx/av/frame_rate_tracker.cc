#include "frame_rate_tracker.h"

#include <numeric>

namespace av {

FrameRateTracker::FrameRateTracker(uint32_t max_frame_history)
    : max_frame_history_(max_frame_history) {
  this->last_frame_start_ = std::chrono::high_resolution_clock::now();
}

int32_t FrameRateTracker::get_fps() const {
  if (this->frame_durations_.empty()) {
    return 0;
  }

  auto result =
      std::reduce(this->frame_durations_.begin(), this->frame_durations_.end());
  return this->frame_durations_.size() / result;
}

void FrameRateTracker::notify_frame_start() {
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> frame_duration =
      std::chrono::duration_cast<std::chrono::duration<double>>(
          now - this->last_frame_start_);
  this->last_frame_start_ = now;
  this->frame_durations_.push_back(frame_duration.count());
  if (this->frame_durations_.size() > this->max_frame_history_) {
    this->frame_durations_.pop_front();
  }
}

}  // namespace av