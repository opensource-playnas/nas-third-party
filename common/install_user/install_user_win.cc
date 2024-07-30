#include "install_user.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "nas/common/nas_config.h"

#include <windows.h>

#include <sddl.h>

namespace nas {

std::string GetSid(unsigned short level) {
  NasConf nas_conf;
  ReadNasConf(2, nas_conf);

  return nas_conf.sid;
}

InstallUserToken::InstallUserToken(unsigned short level) {
  OpenToken(level);
}

InstallUserToken::~InstallUserToken() {
  if (token_ != NULL) {
    CloseHandle(token_);
  }
}

HANDLE InstallUserToken::GetToken() const {
  return token_;
}

void InstallUserToken::OpenToken(unsigned short level) {
  std::wstring sidw = base::UTF8ToWide(GetSid(level));

  // 将SID字符串转换为SID结构
  PSID sid = nullptr;
  if (!ConvertStringSidToSidW(sidw.c_str(), &sid)) {
    LOG(ERROR) << ", sid convert failed";
    return;
  }

  // 获取SID对应的用户信息
  wchar_t account_name[256];
  DWORD account_name_size = sizeof(account_name) / sizeof(wchar_t);
  wchar_t domain_name[256];
  DWORD domain_name_size = sizeof(domain_name) / sizeof(wchar_t);
  SID_NAME_USE sid_name_use;

  if (!LookupAccountSidW(nullptr, sid, account_name, &account_name_size,
                         domain_name, &domain_name_size, &sid_name_use)) {
    LOG(ERROR) << ", LookupAccountSidW failed. Error: " << GetLastError();
    LocalFree(sid);
    return;
  }

  // 打开用户的访问令牌
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token_)) {
    LOG(ERROR) << ", OpenProcessToken failed. Error: " << GetLastError();
    LocalFree(sid);
    return;
  }

  // 清理资源
  LocalFree(sid);
}

};  // namespace nas
