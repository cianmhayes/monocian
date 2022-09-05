#ifndef CXX_BASE_ASYNC_PROCESSING_QUEUE_H_
#define CXX_BASE_ASYNC_PROCESSING_QUEUE_H_

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

namespace base {

template <typename T>
class AsyncProcessingQueueBase {
 public:
  AsyncProcessingQueueBase() = default;
  virtual ~AsyncProcessingQueueBase() = default;
  AsyncProcessingQueueBase(const AsyncProcessingQueueBase&) = delete;
  AsyncProcessingQueueBase& operator=(const AsyncProcessingQueueBase&) = delete;

  void Start() {
    if (!running_) {
      running_ = Startup();
      if (!running_)
        return;
      processing_thread_ = std::thread([this] { ProcessingThread(); });
    }
  }

  void Add(T&& item) {
    {
      std::unique_lock<decltype(m_)> lock(m_);
      q_.push_back(std::move(item));
    }
    cv_.notify_one();
  };

  void Stop() {
    running_ = false;
    cv_.notify_one();
    if (processing_thread_.joinable()) {
      processing_thread_.join();
    }
    Shutdown();
  };

 protected:
  virtual bool Startup() { return true; };

  virtual void Shutdown(){};

  virtual void ProcessItem(T&& item) = 0;

  void ProcessingThread() {
    while (running_) {
      std::list<std::vector<uint8_t>> popped;
      {
        std::unique_lock<decltype(m_)> lock(m_);
        cv_.wait(lock, [this] { return !q_.empty() || !running_; });
        if (!q_.empty()) {
          popped.splice(popped.begin(), q_, q_.begin());
        }
      }
      ProcessItem(std::move(popped.front()));
    }
  }

  bool running_ = false;
  std::list<T> q_;
  mutable std::mutex m_;
  std::condition_variable cv_;
  std::thread processing_thread_;
};

template <typename T>
class AsyncProcessingQueue : AsyncProcessingQueueBase<T> {
 public:
  AsyncProcessingQueue(std::function<void(T&&)> processing_function)
      : processing_function_(processing_function){};
  ~AsyncProcessingQueue() override{};
  AsyncProcessingQueue(const AsyncProcessingQueue&) = delete;
  AsyncProcessingQueue& operator=(const AsyncProcessingQueue&) = delete;

 protected:
  void ProcessItem(T&& item) override { processing_function_(item); };

  std::function<void(T&&)> processing_function_;
};

}  // namespace base

#endif  // CXX_BASE_ASYNC_PROCESSING_QUEUE_H_