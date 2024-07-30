// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-11-30 15:28:29

#ifndef NAS_COMMON_PATH_PROFILE_DEF_H_
#define NAS_COMMON_PATH_PROFILE_DEF_H_

#include <vector>
#include <memory>
#include <string>

namespace nas {
namespace path_unit {

enum class ProfileType : int {
  kUnknownType = 0,
  kDesktop,
  kDownloads,
  kMusic,
  kPictures,
  kVideo
};

struct ProfileInfo {
  ProfileType type = ProfileType::kUnknownType;
  std::string path;
};
using ProfileInfoPtr = std::shared_ptr<ProfileInfo>;
using ProfileInfoArr = std::vector<ProfileInfoPtr>;

struct AccountProfileInfo {
  std::wstring sid;
  bool is_normal = true;  // 是否是一般账户, 除开 administrator 等之外的
  bool monitor_changed = true;  // 是否需要监控其改变( windows 监控注册表的改变)
  ProfileInfoArr profile_list;
};
using AccountProfileInfoPtr = std::shared_ptr<AccountProfileInfo>;
using AccountProfileInfoArr = std::vector<AccountProfileInfoPtr>;

}  // namespace path_unit
}  // namespace nas

#endif