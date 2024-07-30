#include "file_path_unit.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#include <WinIoCtl.h>
#include <Windows.h>
#include <aclapi.h>
#include <sddl.h>
#endif

#include <iostream>
#include <map>

#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/path_service.h"
#if BUILDFLAG(IS_WIN)
#include "base/strings/string_util_win.h"
#endif

#include "nas/common/nas_config.h"
#if BUILDFLAG(IS_WIN)
#include "win_utils.h"
#endif

namespace nas {

namespace path_unit {
static std::map<std::string, FileType> FileTypeMap = {
    {"txt", FileType::kTxt},

    {"pdf", FileType::kPdf},

    {"docx", FileType::kWord},    {"doc", FileType::kWord},
    {"docm", FileType::kWord},    {"dotm", FileType::kWord},
    {"dotx", FileType::kWord},

    {"pptx", FileType::kPPT},     {"ppsx", FileType::kPPT},
    {"ppt", FileType::kPPT},      {"pps", FileType::kPPT},
    {"pptm", FileType::kPPT},     {"potm", FileType::kPPT},
    {"ppam", FileType::kPPT},     {"potx", FileType::kPPT},
    {"ppsm", FileType::kPPT},

    {"xlsx", FileType::kXlxs},    {"xlsb", FileType::kXlxs},
    {"xls", FileType::kXlxs},     {"xlsm", FileType::kXlxs},

    {"html", FileType::kHtml},

    {"7z", FileType::kCompress},  {"zip", FileType::kCompress},
    {"rar", FileType::kCompress}, {"gz", FileType::kCompress},
    {"xz", FileType::kCompress}, {"tar", FileType::kCompress},

    {"3fr", FileType::kPicture},  {"ai", FileType::kPicture},
    {"apng", FileType::kPicture}, {"arw", FileType::kPicture},
    {"avif", FileType::kPicture}, {"bmp", FileType::kPicture},
    {"cr2", FileType::kPicture},  {"crw", FileType::kPicture},
    {"dcr", FileType::kPicture},  {"erf", FileType::kPicture},
    {"gif", FileType::kPicture},  {"heic", FileType::kPicture},
    {"ico", FileType::kPicture},  {"jp2", FileType::kPicture},
    {"jpeg", FileType::kPicture}, {"jpe", FileType::kPicture},
    {"jpg", FileType::kPicture},  {"jpx", FileType::kPicture},
    {"jxr", FileType::kPicture},  {"k25", FileType::kPicture},
    {"kdc", FileType::kPicture},  {"mos", FileType::kPicture},
    {"mrw", FileType::kPicture},  {"nef", FileType::kPicture},
    {"orf", FileType::kPicture},  {"pef", FileType::kPicture},
    {"png", FileType::kPicture},  {"psd", FileType::kPicture},
    {"ptx", FileType::kPicture},  {"raf", FileType::kPicture},
    {"raw", FileType::kPicture},  {"rw2", FileType::kPicture},
    {"sr2", FileType::kPicture},  {"srf", FileType::kPicture},
    {"svg", FileType::kPicture},  {"tga", FileType::kPicture},
    {"tif", FileType::kPicture},  {"tiff", FileType::kPicture},
    {"webp", FileType::kPicture},

    {"mp4", FileType::kVideo},    {"flv", FileType::kVideo},
    {"webm", FileType::kVideo},   {"mov", FileType::kVideo},
    {"3gp", FileType::kVideo},    {"3g2", FileType::kVideo},
    {"rm", FileType::kVideo},     {"rmvb", FileType::kVideo},
    {"wmv", FileType::kVideo},    {"avi", FileType::kVideo},
    {"asf", FileType::kVideo},    {"mpg", FileType::kVideo},
    {"mpeg", FileType::kVideo},   {"ts", FileType::kVideo},
    {"tp", FileType::kVideo},     {"tpr", FileType::kVideo},
    {"div", FileType::kVideo},    {"divx", FileType::kVideo},
    {"dvr-ms", FileType::kVideo}, {"vob", FileType::kVideo},
    {"mkv", FileType::kVideo},    {"mts", FileType::kVideo},
    {"m2ts", FileType::kVideo},   {"m2t", FileType::kVideo},
    {"m4v", FileType::kVideo},    {"qt", FileType::kVideo},
    {"swf", FileType::kVideo},    {"xvvid", FileType::kVideo},
    {"trp", FileType::kVideo},

    {"mp3", FileType::kMusic},    {"m4a", FileType::kMusic},
    {"m4b", FileType::kMusic},    {"aac", FileType::kMusic},
    {"ogg", FileType::kMusic},    {"wav", FileType::kMusic},
    {"flac", FileType::kMusic},   {"ape", FileType::kMusic},
    {"aiff", FileType::kMusic},   {"aif", FileType::kMusic},
    {"wma", FileType::kMusic},

    {"apk", FileType::kApk},      {"exe", FileType::kExe},
    {"torrent", FileType::kBt},

};

base::FilePath UnifiedSlash(const base::FilePath& src_path) {
  base::FilePath::StringType unified_slash_src_path;
  base::ReplaceChars(src_path.value(), FILE_PATH_LITERAL("\\"),
                     FILE_PATH_LITERAL("/"), &unified_slash_src_path);
  return base::FilePath(unified_slash_src_path);
}

std::string UnifiedSlash(const std::string& src_path) {
  std::string unified_slash_src_path;
  base::ReplaceChars(src_path, "\\", "/", &unified_slash_src_path);
  return unified_slash_src_path;
}

base::FilePath RemoveDoubleSlash(const base::FilePath& src_path) {
  base::FilePath::StringType unified_slash_src_path;
  base::ReplaceChars(src_path.value(), FILE_PATH_LITERAL("//"),
                     FILE_PATH_LITERAL("/"), &unified_slash_src_path);
  return base::FilePath(unified_slash_src_path);
}

std::string RemoveDoubleSlash(const std::string& src_path) {
  std::string unified_slash_src_path;
  base::ReplaceChars(src_path, "//", "/", &unified_slash_src_path);
  return unified_slash_src_path;
}

std::string ToWindowsSlash(const std::string& src_path) {
  std::string window_path;
  base::ReplaceChars(src_path, "/", "\\", &window_path);
  return window_path;
}

base::FilePath ToWindowsSlash(const base::FilePath& src_path) {
  base::FilePath::StringType windows_path;
  base::ReplaceChars(src_path.value(), FILE_PATH_LITERAL("/"),
                     FILE_PATH_LITERAL("\\"), &windows_path);
  return base::FilePath(windows_path);
}

base::FilePath GetNoExistFileNameInSpecDir(const base::FilePath& exist_path) {
  base::FilePath no_exist_path;
  base::FilePath temp_path;
  no_exist_path = exist_path;
  temp_path = exist_path;

  int times = 1;
  while (1) {
    if (base::PathExists(temp_path)) {
      if (times > 1) {
        base::FilePath::StringType suffix_number;
        suffix_number = base::StringPrintf(FILE_PATH_LITERAL(" (%d)"), times);
        temp_path = no_exist_path;
        temp_path = temp_path.InsertBeforeExtension(suffix_number);
      } else {
        base::FilePath::StringType suffix = FILE_PATH_LITERAL(" - 副本");
        no_exist_path = no_exist_path.InsertBeforeExtension(suffix);
        temp_path = no_exist_path;
      }
      times++;

    } else {
      // 一定会发生
      no_exist_path = temp_path;
      break;
    }
  }

  return no_exist_path;
}

base::FilePath GetNoExistFilePath(const base::FilePath& exist_path,
                                  bool only_number) {
  if (only_number) {
    base::FilePath return_path = exist_path;
    int times = 2;
    while (base::PathExists(return_path)) {
      base::FilePath::StringType suffix_number;
      suffix_number = base::StringPrintf(FILE_PATH_LITERAL(" (%d)"), times);
      return_path = exist_path.InsertBeforeExtension(suffix_number);
      ++times;
    }
    return return_path;
  } else {
    return nas::path_unit::GetNoExistFileNameInSpecDir(exist_path);
  }
}

void RemoveLastSlash(std::string& path) {
  if (path.length() > 1) {
    if (path[path.length() - 1] == '/' || path[path.length() - 1] == '\\') {
      path.erase(path.end() - 1);
    }
  }
}

base::FilePath AddExtension(const base::FilePath& src_path, const std::string& ext) { 
  base::FilePath::StringType ext_string; 
#if BUILDFLAG(IS_WIN)
   ext_string =  base::UTF8ToWide(ext);
#elif BUILDFLAG(IS_POSIX)
  ext_string = ext;
#else  // TODO other
#error Unsupported platform
#endif
  return src_path.AddExtension(ext_string);
}

base::FilePath BaseFilePathFromU8(const std::string& path, bool convert_slash) {
#if BUILDFLAG(IS_WIN)
  return base::FilePath::FromUTF8Unsafe(convert_slash ? UnifiedSlash(path)
                                                      : path);
#elif BUILDFLAG(IS_POSIX)
  return base::FilePath(convert_slash ? UnifiedSlash(path) : path);
#else  // TODO other
#error Unsupported platform
#endif
  return base::FilePath();
}

base::FilePath BaseFilePathFromU8EX(const std::string& path,
                                    bool convert_slash) {
#if BUILDFLAG(IS_WIN)
  base::FilePath base_file_path = base::FilePath::FromUTF8Unsafe(path);
  if (base_file_path.value().length() >= MAX_PATH - 1) {
    return base::FilePath(LR"(\\?\)" + ToWindowsSlash(base_file_path).value());
  } else {
    return convert_slash ? UnifiedSlash(base_file_path) : base_file_path;
  }
#elif BUILDFLAG(IS_POSIX)
  return BaseFilePathFromU8(path, convert_slash);
#else  // TODO other
#error Unsupported platform
#endif
  return base::FilePath();
}

std::string BaseFilePathToU8(const base::FilePath& path) {
#if BUILDFLAG(IS_WIN)
  return path.AsUTF8Unsafe();
#elif BUILDFLAG(IS_POSIX)
  return path.value();
#else  // TODO other
#error Unsupported platform
#endif
  return "";
}

void CorrectionPath(base::FilePath& path) {
#if BUILDFLAG(IS_WIN)
  if (path.value().length() >= MAX_PATH - 1) {
    path = base::FilePath(LR"(\\?\)" + ToWindowsSlash(path).value());
  }
#endif
}

FileType GetFileType(const std::string& file_name, bool convert_to_lower) {
  nas::path_unit::FileType file_type = nas::path_unit::FileType::kUndefine;
  do {
    size_t pos = file_name.find_last_of(".");
    if (pos == std::string::npos) {
      break;
    }

    std::string suffix = file_name.substr(pos + 1);
    // 转换小写
    std::string lower_suffix =
        convert_to_lower ? base::ToLowerASCII(suffix) : suffix;
    auto iter = FileTypeMap.find(lower_suffix);
    if (iter != FileTypeMap.end()) {
      file_type = iter->second;
    }
  } while (0);

  return file_type;
}

#if BUILDFLAG(IS_WIN)
int AddAccessToNetWorkService(const base::FilePath& dir) {
  // 获取目录的安全描述符
  DWORD dwRes = 0;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  EXPLICIT_ACCESS ea;
  if (GetNamedSecurityInfo(const_cast<LPWSTR>(dir.value().c_str()),
                           SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr,
                           nullptr, &pOldDACL, nullptr,
                           &pSD) != ERROR_SUCCESS) {
    // 处理错误
    return 1;
  }

  // 添加一个访问控制项
  ZeroMemory(&ea, sizeof(EXPLICIT_ACCESSW));
  ea.grfAccessPermissions = GENERIC_ALL;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
  ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
  ea.Trustee.ptstrName = L"NETWORK SERVICE";

  PACL pNewAcl = nullptr;
  if (SetEntriesInAcl(1, &ea, pOldDACL, &pNewAcl) != ERROR_SUCCESS) {
    // 处理错误
    LocalFree(pSD);
    return 1;
  }

  // 更新目录的安全设置
  if (SetNamedSecurityInfo(const_cast<LPWSTR>(dir.value().c_str()),
                           SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr,
                           nullptr, pNewAcl, nullptr) != ERROR_SUCCESS) {
    // 处理错误
    LocalFree(pSD);
    LocalFree(pNewAcl);
    return 1;
  }

  // 释放内存
  LocalFree(pSD);
  LocalFree(pNewAcl);
  return 0;
}

bool HasNetworkServiceReadWriteAccess(const std::wstring& folderPath) {
  PACL pDacl = nullptr;
  PSECURITY_DESCRIPTOR pSd = nullptr;
  SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

  // 获取文件夹的安全描述符
  DWORD result = GetNamedSecurityInfoW(folderPath.c_str(), SE_FILE_OBJECT, si,
                                       nullptr, nullptr, &pDacl, nullptr, &pSd);
  if (result != ERROR_SUCCESS) {
    LOG(ERROR) << L"GetNamedSecurityInfoW failed: " << result;
    return false;
  }

  // 获取 "Network Service" 账户的 SID
  PSID pSid = nullptr;
  SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
  if (!AllocateAndInitializeSid(&siaNtAuthority, 1,
                                SECURITY_NETWORK_SERVICE_RID, 0, 0, 0, 0, 0, 0,
                                0, &pSid)) {
    LOG(ERROR) << L"AllocateAndInitializeSid failed: " << GetLastError();
    LocalFree(pSd);
    return false;
  }

  // 获取 "Network Service" 账户的有效权限
  TRUSTEE trustee;
  BuildTrusteeWithSid(&trustee, pSid);
  DWORD effectiveRights = 0;
  result = GetEffectiveRightsFromAcl(pDacl, &trustee, &effectiveRights);
  if (result != ERROR_SUCCESS) {
    LOG(ERROR) << L"GetEffectiveRightsFromAcl failed: " << result;
    FreeSid(pSid);
    LocalFree(pSd);
    return false;
  }

  bool hasReadAccess =
      (effectiveRights & FILE_GENERIC_READ) == FILE_GENERIC_READ;
  bool hasWriteAccess =
      (effectiveRights & FILE_GENERIC_WRITE) == FILE_GENERIC_WRITE;

  // 释放资源
  FreeSid(pSid);
  LocalFree(pSd);

  return hasReadAccess && hasWriteAccess;
}

// 根据磁盘盘符获取硬件BUG总线
int32_t GetBusType(char letter) {
  bool bResult = true;
  INT BusType = BusTypeUnknown;
  HANDLE hDrv = INVALID_HANDLE_VALUE;
  STORAGE_DEVICE_DESCRIPTOR DevDesc;
  STORAGE_PROPERTY_QUERY ProQuery;

  TCHAR tszSymbol[MAX_PATH] = {TEXT("\\\\?\\*:")};
  ULONG ulBytesResults = 0;
  memset(&ProQuery, 0, sizeof(ProQuery));
  memset(&DevDesc, 0, sizeof(DevDesc));

  if ((letter >= TEXT('A') && letter <= TEXT('Z')) ||
      (letter >= TEXT('a') && letter <= TEXT('z'))) {
  } else {
    return false;
  }

  tszSymbol[4] = letter;

  do {
    hDrv =
        CreateFile(tszSymbol, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hDrv)
      break;

    ProQuery.QueryType = PropertyStandardQuery;
    ProQuery.PropertyId = StorageDeviceProperty;
    if (!DeviceIoControl(hDrv, IOCTL_STORAGE_QUERY_PROPERTY, &ProQuery,
                         sizeof(ProQuery), &DevDesc, sizeof(DevDesc),
                         &ulBytesResults, NULL)) {
      break;
    }
    BusType = DevDesc.BusType;
  } while (0);

  if (hDrv != INVALID_HANDLE_VALUE) {
    CloseHandle(hDrv);
    hDrv = INVALID_HANDLE_VALUE;
  }

  return BusType;
}

bool IsUSB(char letter) {
  return GetBusType(letter) == BusTypeUsb;
}

#endif

bool MatchPath(const std::string& path1,
               const std::string& path2,
               bool full_equal) {
  if (path1.length() < path2.length()) {
    return false;
  }
  if (full_equal) {
    if (path1.length() != path2.length()) {
      return false;
    }
  }

#if BUILDFLAG(IS_WIN)
  return base::StartsWith(path1, path2, base::CompareCase::INSENSITIVE_ASCII);
#else
  return base::StartsWith(path1, path2, base::CompareCase::SENSITIVE);
#endif
}

void GetProfileList(AccountProfileInfoArr& profile_list) {
#if BUILDFLAG(IS_WIN)
  GetProfileListWin(profile_list);
#endif
}

}  // namespace path_unit
}  // namespace nas
