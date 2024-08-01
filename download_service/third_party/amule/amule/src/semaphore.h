#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include <condition_variable>
#include <mutex>
#include <chrono>

class Semaphore {
public:
  explicit Semaphore(int count = 0) : count_(count) {}

  void Signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [=] { return count_ > 0; });
    --count_;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};
#endif
