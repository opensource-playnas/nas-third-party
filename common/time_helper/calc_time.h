
#include <string>

#include "base/time/time.h"

#ifndef NAS_COMMON_TIME_HELPER_CALC_TIME_H_
#define NAS_COMMON_TIME_HELPER_CALC_TIME_H_

// progress取值范围为 0 - 1
// used_time 是已消耗的时间
std::string CalcRemainingTime(const base::TimeDelta& used_time,
                                float progress);

#endif //NAS_COMMON_TIME_HELPER_CALC_TIME_H_