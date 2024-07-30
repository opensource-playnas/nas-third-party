// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-11-28 17:20:36

#ifndef NAS_COMMON_PATH_PATH_PROTECTED_H_
#define NAS_COMMON_PATH_PATH_PROTECTED_H_

#include <string>
#include <list>
#include <memory>
#include <thread>

#include "base/memory/singleton.h"

#include "nas/common/nas_thread.h"
#include "profile_def.h"

namespace nas {
namespace path_unit {

using ProfilePathArr = std::vector<std::string>;

#if BUILDFLAG(IS_WIN)
// windows 下 profile 的获取 和 重定向监控
// profile path 路径本省不能被更改, 但是其 children 可被改
// 比如: C:\users\user_name\Desktop, 不能被删除重命名,
// 但是  C:\users\user_name\Desktop\sub.txt 可被更改
// 所以这里特殊的拎出来
class ProfileFile {
  struct MonitorRegInfo {
    HKEY hkey = NULL;
    HANDLE hevent = NULL;
    std::wstring sid;
  };
  using MonitorRegInfoArr = std::vector<MonitorRegInfo>;

 public:
  void Init();
  void Uninit();
  void GetAllProfilePath(ProfilePathArr& profile_path_list);
  void GetProfileInfo(const std::wstring& sid, ProfileInfoArr& profile_list);

 private:
  void StartMonitorReg();
  void StopMonitorReg();
  void MonitorRegThread();

  void UpdateAccountProfile(const std::wstring& sid,
                            ProfileInfoArr profile_list);

  void UnifiedSlash();
  static void UnifiedSlash(ProfileInfoArr& profile_list);

 private:
  HANDLE hexit_ = NULL;
  std::unique_ptr<std::thread> monitor_thread_;
  MonitorRegInfoArr monitor_reg_;

  base::Lock account_profile_lock_;
  AccountProfileInfoArr account_profile_;
};

using ProfileFilePtr = std::unique_ptr<ProfileFile>;
#endif

// 只读配置, 这里使用了单例
// 外部调用, 通过 ProtectedPathCheck 更方便
class ProtectedPath {
 public:
  static ProtectedPath* GetInstance();

  void Init();
  void Uninit();
  // path_u8 属于被保护路径 或者 是被保护路径子文件
  bool IsProtected(const std::string& path_u8,
                   const ProfilePathArr& profile_path_list) const;
  // 在 IsProtected 的基础上, path_u8 是被保护路径的长辈, 也被保护
  bool IsProtectedEx(const std::string& path_u8,
                     const ProfilePathArr& profile_path_list) const;

#if BUILDFLAG(IS_WIN)
  const ProfileFilePtr& Profile() const { return profile_; }
#endif

 private:
  ProtectedPath() = default;
  ~ProtectedPath() = default;
  ProtectedPath(const ProtectedPath&) = delete;
  ProtectedPath& operator=(const ProtectedPath&) = delete;
  friend struct base::DefaultSingletonTraits<ProtectedPath>;

 private:
  std::list<std::string> protected_paths_;
#if BUILDFLAG(IS_WIN)
  ProfileFilePtr profile_;
#endif
};

class ProtectedPathCheck {
 public:
  // 检测全路径, 使用该构造
  ProtectedPathCheck();
  // 需检测子文件, 可以使用该构造
  ProtectedPathCheck(const std::string& path_u8);

  // path 属于被保护路径 或者 是被保护路径子文件
  bool IsProtected() const;
  bool IsProtected(const std::string& sub_file_name_u8) const;
  bool IsPathProtected(const std::string& path_u8) const;
  // 指定路径下的内容是否被保护, 不判断路径本身
  // profile 路径 就是本身不能被更改, 但是路径下的文件能被更改
  bool IsProtectedInPath(const std::string& path_u8) const;
  // 在 IsProtected 的基础上, path 是被保护路径的长辈 也被保护
  bool IsProtectedEx() const;
  bool IsProtectedEx(const std::string& sub_file_name_u8) const;
  bool IsPathProtectedEx(const std::string& path_u8) const;

  bool IsProfilePath(const std::string& path_u8) const;

 private:
  ProfilePathArr profile_path_list_;
  std::string root_path_; // 要检测的路径的根路径, 正斜杠 /
  bool root_is_profile_ = false;
  bool root_is_protected_ = true;
};
using ProtectedPathCheckPtr = std::shared_ptr<ProtectedPathCheck>;

}  // namespace path_unit
}  // namespace nas

#endif 