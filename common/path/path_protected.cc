// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-11-28 17:21:14

#include "path_protected.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#endif

#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/path_service.h"

#include "nas/common/nas_config.h"
#include "file_path_unit.h"
#if BUILDFLAG(IS_WIN)
#include "win_utils.h"
#endif


namespace nas {
namespace path_unit {

static bool IsStartWith(const std::string& path, const std::string& start) {
  return path.length() == start.length() ? MatchPath(path, start, false)
                                         : MatchPath(path, start + "/", false);
}

static bool IsProfilePath(const std::string& path_u8,
                          const ProfilePathArr& profile_path_list) {
  for (const auto& path : profile_path_list) {
    if (MatchPath(path_u8, path, true)) {
      return true;
    }
  }
  return false;
}

static bool IsStartProfilePath(const std::string& path_u8,
                               const ProfilePathArr& profile_path_list) {
  for (const auto& path : profile_path_list) {
    if (IsStartWith(path_u8, path)) {
      return true;
    }
  }
  return false;
}

#if BUILDFLAG(IS_WIN)
void ProfileFile::Init() {
  nas::path_unit::GetProfileList(account_profile_);
  UnifiedSlash();
  StartMonitorReg();

  for (const auto& account_profile : account_profile_) {
    if (account_profile) {
      LOG(INFO) << "sid(profile path): " << account_profile->sid;
      for (const auto& profile : account_profile->profile_list) {
        LOG(INFO) << "profile path: " << profile->path;
      }
    }
  }
}

void ProfileFile::Uninit() {
  StopMonitorReg();
}

void ProfileFile::GetAllProfilePath(ProfilePathArr& profile_path_list) {
  base::AutoLock auto_lock(account_profile_lock_);
  if (account_profile_.empty()) {
    Init();
  }
  for (const auto& account_profile : account_profile_) {
    if (account_profile) {
      for (const auto& profile : account_profile->profile_list) {
        profile_path_list.push_back(profile->path);
      }
    }
  }
}

void ProfileFile::GetProfileInfo(const std::wstring& sid,
                                 ProfileInfoArr& profile_list) {
  base::AutoLock auto_lock(account_profile_lock_);
  if (account_profile_.empty()) {
    Init();
  }
  for (const auto& account_profile : account_profile_) {
    if (account_profile && account_profile->sid == sid) {
      profile_list = account_profile->profile_list;
      break;
    }
  }
}

void ProfileFile::StartMonitorReg() {
  // 筛选需要监视的账户
  for (const auto& account_profile : account_profile_) {
    if (account_profile->is_normal && account_profile->monitor_changed) {
      const std::wstring key_path =
          account_profile->sid +
          L"\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\"
          L"Explorer\\Shell Folders";
      // KEY_READ, 包含了 KEY_NOTIFY
      HKEY hkey = NULL;
      LONG lresult =
          RegOpenKeyExW(HKEY_USERS, key_path.c_str(), 0, KEY_READ, &hkey);
      if (lresult == ERROR_SUCCESS) {
        HANDLE hevent = ::CreateEventW(NULL, TRUE, FALSE, NULL);
        monitor_reg_.push_back({hkey, hevent, account_profile->sid});
      } else {
        LOG(WARNING) << "monitor reg, open reg failed, err: " << lresult;
      }
    }
  }

  if (!monitor_reg_.empty()) {
    hexit_ = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    monitor_thread_ =
        std::make_unique<std::thread>(&ProfileFile::MonitorRegThread, this);
  }
}

void ProfileFile::StopMonitorReg() {
  if (hexit_) {
    ::SetEvent(hexit_);
  }

  if (monitor_thread_) {
    monitor_thread_->join();
  }

  if (hexit_) {
    ::CloseHandle(hexit_);
    hexit_ = NULL;
  }

  for (auto& monitor_reg : monitor_reg_) {
    if (monitor_reg.hevent) {
      ::CloseHandle(monitor_reg.hevent);
      monitor_reg.hevent = NULL;
    }
    if (monitor_reg.hkey) {
      ::RegCloseKey(monitor_reg.hkey);
      monitor_reg.hkey = NULL;
    }
  }
}

// TODO 未登录的用户登录, 还需监控 HKEY_USER
void ProfileFile::MonitorRegThread() {
  std::vector<HANDLE> events;

  for (auto& monitor_reg : monitor_reg_) {
    events.push_back(monitor_reg.hevent);
    RegNotifyChangeKeyValue(monitor_reg.hkey, TRUE, REG_NOTIFY_CHANGE_LAST_SET,
                            monitor_reg.hevent, TRUE);
  }

  while (!(WaitForSingleObject(hexit_, 0) == WAIT_OBJECT_0)) {
    DWORD wait_ret =
        WaitForMultipleObjects(events.size(), events.data(), FALSE, 2000);
    if (wait_ret >= WAIT_OBJECT_0 && wait_ret < WAIT_OBJECT_0 + events.size()) {
      DWORD index = wait_ret - WAIT_OBJECT_0;
      ResetEvent(events[index]);
      nas::path_unit::ProfileInfoArr profile_list;
      nas::path_unit::GetProfileList(monitor_reg_[index].hkey, profile_list);
      UpdateAccountProfile(monitor_reg_[index].sid, profile_list);
      RegNotifyChangeKeyValue(monitor_reg_[index].hkey, TRUE,
                              REG_NOTIFY_CHANGE_LAST_SET,
                              monitor_reg_[index].hevent, TRUE);
    }
  }
}

void ProfileFile::UpdateAccountProfile(const std::wstring& sid,
                                       ProfileInfoArr profile_list) {
  base::AutoLock auto_lock(account_profile_lock_);
  for (auto& account_profile : account_profile_) {
    if (account_profile && account_profile->sid == sid) {
      account_profile->profile_list = profile_list;
      UnifiedSlash(account_profile->profile_list);
      break;
    }
  }
}

void ProfileFile::UnifiedSlash() {
  for (auto& account_profile : account_profile_) {
    ProfileFile::UnifiedSlash(account_profile->profile_list);
  }
}

void ProfileFile::UnifiedSlash(ProfileInfoArr& profile_list) {
  for (auto& profile : profile_list) {
    profile->path = nas::path_unit::UnifiedSlash(profile->path);
  }
}

#endif

ProtectedPath* ProtectedPath::GetInstance() {
  return base::Singleton<ProtectedPath>::get();
}

void ProtectedPath::Init() {
#if BUILDFLAG(IS_WIN)
  profile_ = std::make_unique<ProfileFile>();
  profile_->Init();

  base::FilePath dir;
  base::PathService::Get(base::DIR_WINDOWS, &dir);
  protected_paths_.push_back(BaseFilePathToU8(dir));

  base::PathService::Get(base::DIR_PROGRAM_FILESX86, &dir);
  protected_paths_.push_back(BaseFilePathToU8(dir));

  base::PathService::Get(base::DIR_PROGRAM_FILES6432, &dir);
  protected_paths_.push_back(BaseFilePathToU8(dir));

  base::PathService::Get(base::DIR_COMMON_APP_DATA, &dir);
  protected_paths_.push_back(BaseFilePathToU8(dir));

  // 通过 DIR_HOME, 也就是 CSIDL_PROFILE
  // system 账户下获取的路径是 C:\Windows\system32\config
  // 所以通过 public desktop 获取(public desktop 不能被重定向)
  base::PathService::Get(base::DIR_COMMON_DESKTOP, &dir);
  protected_paths_.push_back(BaseFilePathToU8(dir.DirName().DirName()));
#endif

  protected_paths_.push_back(BaseFilePathToU8(GetNasAppInstallDir()));
  protected_paths_.push_back(BaseFilePathToU8(GetNasHomeDir()));
  protected_paths_.push_back(BaseFilePathToU8(GetAppConfigDir()));
  protected_paths_.push_back(BaseFilePathToU8(GetGlobalConfigDir()));

  for (auto& path : protected_paths_) {
    path = nas::path_unit::UnifiedSlash(path);
    LOG(INFO) << "protected path: " << path;
  }
}

void ProtectedPath::Uninit() {
#if BUILDFLAG(IS_WIN)
  profile_->Uninit();
#endif
  protected_paths_.clear();
}

bool ProtectedPath::IsProtected(const std::string& path_u8,
                                const ProfilePathArr& profile_path_list) const {
  if (IsProfilePath(path_u8, profile_path_list)) {
    return true;
  }

  if (IsStartProfilePath(path_u8, profile_path_list)) {
    return false;
  }

  for (const auto& path : protected_paths_) {
    if (IsStartWith(path_u8, path)) {
      return true;
    }
  }
  return false;
}

bool ProtectedPath::IsProtectedEx(
    const std::string& path_u8,
    const ProfilePathArr& profile_path_list) const {
  if (IsProtected(path_u8, profile_path_list)) {
    return true;
  }

  for (const auto& path : protected_paths_) {
    if (IsStartWith(path, path_u8)) {
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////
ProtectedPathCheck::ProtectedPathCheck() {
#if BUILDFLAG(IS_WIN)
  if (ProtectedPath::GetInstance()->Profile()) {
    ProtectedPath::GetInstance()->Profile()->GetAllProfilePath(
        profile_path_list_);
  }
#endif
}

ProtectedPathCheck::ProtectedPathCheck(const std::string& path_u8)
    : ProtectedPathCheck() {
  root_path_ = nas::path_unit::UnifiedSlash(path_u8);
  root_is_profile_ = IsProfilePath(root_path_);
  root_is_protected_ =
      ProtectedPath::GetInstance()->IsProtected(root_path_, profile_path_list_);
}

bool ProtectedPathCheck::IsProtected() const {
  return IsPathProtected(root_path_);
}

bool ProtectedPathCheck::IsProtected(
    const std::string& sub_file_name_u8) const {
  if (root_is_profile_) {
    return false;
  } else {
    if (root_is_protected_) {
      return true;
    } else {
      return IsPathProtected(root_path_ + "/" + sub_file_name_u8);
    }
  }
}

bool ProtectedPathCheck::IsPathProtected(const std::string& path_u8) const {
  return ProtectedPath::GetInstance()->IsProtected(path_u8, profile_path_list_);
}

bool ProtectedPathCheck::IsProtectedInPath(const std::string& path_u8) const {
  if (IsProfilePath(path_u8)) {
    return false;
  } else {
    return IsPathProtected(path_u8);
  }
}

bool ProtectedPathCheck::IsProtectedEx() const {
  return IsPathProtectedEx(root_path_);
}

bool ProtectedPathCheck::IsProtectedEx(
    const std::string& sub_file_name_u8) const {
  if (root_is_profile_) {
    return false;
  } else {
    if (root_is_protected_) {
      return true;
    } else {
      return IsPathProtectedEx(root_path_ + "/" + sub_file_name_u8);
    }
  }
}

bool ProtectedPathCheck::IsPathProtectedEx(const std::string& path_u8) const {
  return ProtectedPath::GetInstance()->IsProtectedEx(path_u8,
                                                     profile_path_list_);
}

bool ProtectedPathCheck::IsProfilePath(const std::string& path_u8) const {
  return nas::path_unit::IsProfilePath(path_u8, profile_path_list_);
}

}  // namespace path_unit
}  // namespace nas
