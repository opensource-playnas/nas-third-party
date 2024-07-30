// stream_id.hpp

#ifndef NAS_COMMON_STREAM_ID_HPP_
#define NAS_COMMON_STREAM_ID_HPP_

#include <chrono>
#include <atomic>
#include <string>
#include <sstream>

namespace nas {
/*
* 根据时间戳生成一个唯一的id,相同时间戳再后面递增序号
*/
class StreamId {
public:
    StreamId() : seq_(0) {
        last_ts_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    std::string GetNextIdStr() {
        long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        if (ts == last_ts_) {
            ++seq_;
        } else {
            last_ts_ = ts;
            seq_ = 0;
        }

        std::stringstream ss;
        ss << ts << "_" << seq_;

        return ss.str();
    }

private:
    std::atomic<int> seq_;
    long long last_ts_;
}; // class StreamId

} // namespace nas
#endif // NAS_COMMON_STREAM_ID_HPP_