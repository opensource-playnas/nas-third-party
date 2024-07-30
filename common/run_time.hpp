
#ifndef NAS_MESSAGE_CENTER_RUN_TIME_HPP_
#define NAS_MESSAGE_CENTER_RUN_TIME_HPP_

#include <chrono>
#include <map>

#include "base/guid.h"
#include "base/logging.h"

namespace nas {
class RunTime {
 public:
  void Start() {
    last_tick_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();
    id_ = base::GenerateGUID();
    LOG(INFO) << "id: " << id_ << " start";
  }
  void PrintCost(const std::string& mark, int step) {
    int64_t curr_tick = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    LOG(INFO) << "id:" << id_ << ",mark:" << mark << ", step:" << step
              << ",cost:" << curr_tick - last_tick_ << " ms";
    last_tick_ = curr_tick;
  }

 private:
  int64_t last_tick_ = 0;
  std::string id_;
  // 记录当前mark标记调用的时刻,方便统计时间时不太多的侵入代码
  static std::map<std::string, int64_t> mark_timestamp_;

};  // class RunTime
using RunTimePtr = std::shared_ptr<RunTime>;
}  // namespace nas

#endif  // NAS_MESSAGE_CENTER_RUN_TIME_HPP_