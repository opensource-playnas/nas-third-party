// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-11-30 15:19:07

#include "win_utils.h"

#include <Windows.h>
#include <shlwapi.h>

#include <map>
#include <string>
#include <memory>

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"

#include "file_path_unit.h"

namespace nas {
namespace path_unit {

// 注册表的键值与rpofile type
static const std::map<std::wstring, ProfileType> kProfileKeyType = {
    {L"Desktop", ProfileType::kDesktop},
    {L"{374DE290-123F-4565-9164-39C4925E467B}", ProfileType::kDownloads},
    {L"My Music", ProfileType::kMusic},
    {L"My Pictures", ProfileType::kPictures},
    {L"My Video", ProfileType::kVideo}};

static const std::map<ProfileType, std::wstring> kProfileTypeFolder = {
    {ProfileType::kDesktop, L"Desktop"},
    {ProfileType::kDownloads, L"Downloads"},
    {ProfileType::kMusic, L"Music"},
    {ProfileType::kPictures, L"Pictures"},
    {ProfileType::kVideo, L"Video"}};

struct SIDInfo {
  std::wstring sid;
  std::wstring profile_image_path;
};
using SIDInfoPtr = std::shared_ptr<SIDInfo>;
using SIDInfoArr = std::vector<SIDInfoPtr>;

static void EnablePrivilege(const wchar_t* privilege) {
  HANDLE token;
  TOKEN_PRIVILEGES tp;

  if (OpenProcessToken(GetCurrentProcess(),
                       TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
    LookupPrivilegeValue(NULL, privilege, &tp.Privileges[0].Luid);

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                          (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);

    CloseHandle(token);
  }
}

static std::wstring GetStringValue(HKEY hkey, const wchar_t* name) {
  std::wstring ret;
  do {
    if (!hkey || !name) {
      break;
    }

    DWORD type = REG_EXPAND_SZ;
    DWORD read_size = 0;
    LONG lresult =
        RegQueryValueExW(hkey, name, nullptr, &type, nullptr, &read_size);
    if (ERROR_SUCCESS != lresult) {
      LOG(ERROR) << "reg get length error, " << lresult;
      break;
    }
    if (0 == read_size) {
      break;
    }

    std::vector<wchar_t> buffer(read_size / sizeof(wchar_t));
    lresult =
        RegQueryValueExW(hkey, name, nullptr, &type,
                         reinterpret_cast<LPBYTE>(buffer.data()), &read_size);
    if (ERROR_SUCCESS != lresult) {
      LOG(ERROR) << "reg read string value error, " << lresult;
      break;
    }
    ret = std::wstring(buffer.cbegin(), buffer.cend()).c_str();
  } while (false);

  return ret;
}

// 获取指定 sid 的 profile image path
static std::wstring GetProfileImagePath(HKEY hprofile_list_key,
                                        const std::wstring& sid) {
  std::wstring ret;
  HKEY hsub_key = nullptr;
  if (ERROR_SUCCESS ==
      RegOpenKeyExW(hprofile_list_key, sid.c_str(), 0, KEY_READ, &hsub_key)) {
    ret = GetStringValue(hsub_key, L"ProfileImagePath");
    RegCloseKey(hsub_key);
    hsub_key = nullptr;
  }
  return ret;
}

static void GetAllSidInfo(SIDInfoArr& sid_infos) {
  HKEY hkey = nullptr;
  do {
    const std::wstring profile_list_path =
        LR"(SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList)";
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, profile_list_path.c_str(), 0,
                      KEY_READ, &hkey) != ERROR_SUCCESS) {
      LOG(ERROR) << "open reg key failed";
      break;
    }

    DWORD num_subkeys = 0;
    DWORD max_subkey_len = 0;
    if (RegQueryInfoKeyW(hkey, nullptr, nullptr, nullptr, &num_subkeys,
                         &max_subkey_len, nullptr, nullptr, nullptr, nullptr,
                         nullptr, nullptr) != ERROR_SUCCESS) {
      LOG(ERROR) << "query reg key info failed";
      break;
    }

    max_subkey_len++;  // 考虑到空字符
    std::vector<wchar_t> subkey_name(max_subkey_len);
    for (DWORD i = 0; i < num_subkeys; ++i) {
      DWORD subkey_name_len = max_subkey_len;
      if (RegEnumKeyExW(hkey, i, subkey_name.data(), &subkey_name_len, nullptr,
                        nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        const std::wstring sid(subkey_name.data(), subkey_name_len);
        // S-1-5-18, S-1-5-80-0 之类的属于系统账户
        if (std::count(sid.begin(), sid.end(), L'-') > 4) {
          const std::wstring profile_image_path =
              GetProfileImagePath(hkey, sid);
          if (!profile_list_path.empty()) {
            SIDInfoPtr sid_info(new SIDInfo);
            sid_info->sid = sid;
            sid_info->profile_image_path = profile_image_path;
            sid_infos.push_back(std::move(sid_info));
          }
        }
      }
    }
  } while (0);

  if (hkey) {
    RegCloseKey(hkey);
    hkey = nullptr;
  }
}

// SID 相关内容参考
// https://learn.microsoft.com/en-us/windows-server/identity/ad-ds/manage/understand-security-identifiers
static int64_t GetLastRid(const std::wstring& sid) {
  int64_t rid = 0;

  do {
    if (sid.empty()) {
      break;
    }

    const size_t pos = sid.rfind(L'-');
    if (pos == std::wstring::npos) {
      break;
    }

    if (pos >= sid.length()) {
      break;
    }

    rid = std::stoll(sid.substr(pos + 1));
  } while (false);

  return rid;
}

void GetProfileList(HKEY hkey, ProfileInfoArr& profile_list) {
  if (!hkey) {
    return;
  }
  for (const auto& [key, profile_type] : kProfileKeyType) {
    std::wstring path = GetStringValue(hkey, key.c_str());
    if (path.empty()) {
      continue;
    }
    ProfileInfoPtr profile(new ProfileInfo);
    profile->type = profile_type;
    profile->path = base::WideToUTF8(path);
    profile_list.push_back(std::move(profile));
  }
}

static void GetNormalProfileList(const std::wstring& sid,
                                 ProfileInfoArr& profile_list) {
  HKEY hkey = nullptr;
  do {
    if (sid.empty()) {
      break;
    }

    std::wstring full_key_path =
        sid +
        L"\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\"
        L"Explorer\\Shell Folders";

    LONG lresult =
        RegOpenKeyExW(HKEY_USERS, full_key_path.c_str(), 0, KEY_READ, &hkey);
    if (lresult != ERROR_SUCCESS) {
      LOG(WARNING) << "reg open key: " << sid << " failed, err: " << lresult;
      break;
    }

    GetProfileList(hkey, profile_list);
  } while (false);

  if (hkey) {
    RegCloseKey(hkey);
    hkey = nullptr;
  }
}

// 获取特殊账户的, administrator 等
// 他们的路径无法重定向, 直接获取
static void GetSpecialProfileList(const std::wstring& profile_image_path,
                                  ProfileInfoArr& profile_list) {
  for (const auto& pref_profile : kProfileTypeFolder) {
    ProfileInfoPtr profile_info(new ProfileInfo);
    profile_info->type = pref_profile.first;
    profile_info->path =
        base::WideToUTF8(profile_image_path + L"\\" + pref_profile.second);
    profile_list.push_back(std::move(profile_info));
  }
}

// HKEY_USERS 下不存在的 sid
// 账号不登录, sid 信息就不会存在在 HKEY_USERS 下面
static void GetNotUnderUserProfileList(const SIDInfoPtr& sid_info,
                                       ProfileInfoArr& profile_list) {
  do {
    if (sid_info->sid.empty() || sid_info->profile_image_path.empty()) {
      break;
    }
    LOG(INFO) << "get not under user key, sid: " << sid_info->sid;

    const std::wstring tmp_key_name = L"tmp-" + sid_info->sid;
    const std::wstring ntuser_data_path =
        sid_info->profile_image_path + L"\\NTUSER.DAT";

    if (!::PathFileExists(ntuser_data_path.c_str())) {
      LOG(WARNING) << ntuser_data_path << " not exist";
      break;
    }
    LONG lresult =
        RegLoadKeyW(HKEY_USERS, tmp_key_name.c_str(), ntuser_data_path.c_str());
    if (lresult != ERROR_SUCCESS) {
      LOG(WARNING) << "reg load key failed, err: " << lresult;
      break;
    }

    GetNormalProfileList(tmp_key_name, profile_list);

    lresult = RegUnLoadKeyW(HKEY_USERS, tmp_key_name.c_str());
    if (lresult != ERROR_SUCCESS) {
      LOG(WARNING) << "reg unload key failed, err: " << lresult;
      break;
    }
  } while (false);
}

// 获取指定 sid 的 profile list
static AccountProfileInfoPtr GetSidProfileList(const SIDInfoPtr& sid_info) {
  AccountProfileInfoPtr info(new AccountProfileInfo);
  info->sid = sid_info->sid;
  do {
    if (sid_info->sid.empty()) {
      break;
    }

    // 虚拟账户
    if (sid_info->sid.find(L"S-1-5-82") == 0) {
      break;
    }

    const auto rid = GetLastRid(sid_info->sid);
    if (0 == rid) {
      LOG(WARNING) << "get rid failed, sid: " << sid_info->sid;
      break;
    }

    if (rid >= 500 && rid < 600) {
      info->is_normal = false;
      GetSpecialProfileList(sid_info->profile_image_path, info->profile_list);
      break;
    }

    HKEY hsid_key = nullptr;
    long lresult = RegOpenKeyExW(HKEY_USERS, sid_info->sid.c_str(), 0, KEY_READ,
                                 &hsid_key);
    if (lresult == ERROR_FILE_NOT_FOUND) {
      info->monitor_changed = false;
      GetNotUnderUserProfileList(sid_info, info->profile_list);
      break;
    } else if (lresult != ERROR_SUCCESS) {
      LOG(WARNING) << "reg open key: " << sid_info->sid
                   << " failed, err: " << lresult;
      break;
    }
    RegCloseKey(hsid_key);
    hsid_key = nullptr;

    GetNormalProfileList(sid_info->sid, info->profile_list);
  } while (false);

  return info;
}

void GetProfileListWin(AccountProfileInfoArr& account_profile_list) {
  EnablePrivilege(SE_RESTORE_NAME);
  EnablePrivilege(SE_BACKUP_NAME);
  SIDInfoArr sid_infos;
  GetAllSidInfo(sid_infos);
  for (const auto& sid_info : sid_infos) {
    if (sid_info->sid.empty()) {
      continue;
    }

    AccountProfileInfoPtr info = GetSidProfileList(sid_info);
    if (!info->profile_list.empty()) {
      account_profile_list.push_back(std::move(info));
    }
  }
}

}  // namespace path_unit
}  // namespace nas