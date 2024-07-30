// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/03/29 19:28

#ifndef NAS_COMMON_MID_NAS_MID_H_
#define NAS_COMMON_MID_NAS_MID_H_
#include <string>

namespace nas {
class NasNidHelper {
 public:
  NasNidHelper();
  ~NasNidHelper();

  static bool GetNid(std::string* nid);
};
}  // namespace nas

#endif  // NAS_COMMON_MID_NAS_MID_H_
