
#include "calc_time.h"

#include "base/strings/stringprintf.h"

static constexpr int64_t kDay = 60 * 60 * 24;
static constexpr int64_t kHour = 60 * 60;
static constexpr int64_t kMinute = 60;
std::string CalcRemainingTime(
    const base::TimeDelta& used_time,
    float progress) {
  if (progress < 0.000001) return "--";
  const int64_t used_seconds = used_time.InSeconds();
  int64_t remaining_seconds =
      static_cast<int64_t>(double(used_seconds) / progress) - used_seconds;
  remaining_seconds = std::max(int64_t(0), remaining_seconds);

  int64_t days = remaining_seconds / kDay;
  remaining_seconds = remaining_seconds % kDay;

  int64_t hours = remaining_seconds / kHour;
  remaining_seconds = remaining_seconds % kHour;

  int64_t minutes = remaining_seconds / kMinute;
  remaining_seconds = remaining_seconds % kMinute;

  return base::StringPrintf(
      "%s%02lld:%02lld:%02lld",
      (days > 0 ? base::StringPrintf("%lld天 ", (long long)(days)).c_str()
                : ""),
      (long long)(hours), (long long)(minutes), (long long)(remaining_seconds));
}
