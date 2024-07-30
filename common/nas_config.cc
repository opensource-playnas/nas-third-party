// copyright 2022 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2022/09/14 16:01

#include "nas_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#elif BUILDFLAG(IS_POSIX)  // 获取路径
#include <glib.h>
#endif

#include <string>
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "nas/common/json_cfg/json_cfg.h"
#include "nas/common/path/file_path_unit.h"

namespace nas {

#if BUILDFLAG(IS_WIN)
// On Windows, for Unicode-aware applications, native pathnames are wchar_t
// arrays encoded in UTF-16.
typedef std::wstring NasStringType;
#elif BUILDFLAG(IS_POSIX) || BUILDFLAG(IS_FUCHSIA)
// On most platforms, native pathnames are char arrays, and the encoding
// may or may not be specified.  On Mac OS X, native pathnames are encoded
// in UTF-8.
typedef std::string NasStringType;
#endif  // BUILDFLAG(IS_WIN)

static void ReadNasConf(const base::FilePath& config_path, NasConf& nas_conf);
// 迭代最深目录层数
static const int kMaxDirDeepLevel = 20;
static const NasStringType kInstallEntryFile = FILE_PATH_LITERAL("nas.json");

base::FilePath GetNasHomeDir() {
  base::FilePath cache_path;

#if BUILDFLAG(IS_WIN)
  base::PathService::Get(base::DIR_COMMON_DESKTOP, &cache_path);
  cache_path = cache_path.DirName().Append(FILE_PATH_LITERAL(".home_nas"));

  if (base::PathExists(cache_path)) {
    return cache_path;
  }

#elif BUILDFLAG(IS_POSIX)
  std::string data_dir = base::StringPrintf("%s/nas", g_get_user_data_dir());
  cache_path = base::FilePath(data_dir);
#endif

  base::File::Error err = base::File::FILE_OK;
  if (!base::CreateDirectoryAndGetError(cache_path, &err)) {
    // LOG(ERROR) << "create dir failed: " <<
    // base::File::ErrorToString(err);
    cache_path.clear();
  }
#if BUILDFLAG(IS_WIN)
  std::string path = cache_path.AsUTF8Unsafe();
  ::SetFileAttributes(base::UTF8ToWide(path).c_str(), FILE_ATTRIBUTE_HIDDEN);
#elif BUILDFLAG(IS_POSIX)
  // TODO
#endif
  return cache_path;
}

static base::FilePath NasHomePathAppend(
    base::FilePath::StringPieceType component) {
  base::FilePath path = GetNasHomeDir().Append(component);
  if (!base::PathExists(path)) {
    base::File::Error err = base::File::FILE_OK;
    if (!base::CreateDirectoryAndGetError(path, &err)) {
      // LOG(ERROR) << "create dir failed: " <<
      // base::File::ErrorToString(err);
      path.clear();
    }
  }
  return path;
}

std::string GetNasEntryDir() {
  nas::NasConf nas_conf;
  base::FilePath install_dir = GetNasAppInstallDir();
  base::FilePath nas_json_path = install_dir.Append(kInstallEntryFile);
  if (base::PathExists(nas_json_path)) {
    ReadNasConf(nas_json_path, nas_conf);
  }

  return nas_conf.entry_dir;
}

base::FilePath GetUserDataDir(const base::FilePath& kInstallDir) {}

base::FilePath GetDataDir() {
  nas::NasConf nas_conf;
  base::FilePath install_dir = GetNasAppInstallDir();
  base::FilePath nas_json_path = install_dir.Append(kInstallEntryFile);
  if (base::PathExists(nas_json_path)) {
    ReadNasConf(nas_json_path, nas_conf);
  }

  base::FilePath data_dir;
#if BUILDFLAG(IS_WIN)
  data_dir = base::FilePath(base::UTF8ToWide(nas_conf.user_data_dir));
#else
  data_dir = base::FilePath(nas_conf.user_data_dir);
#endif  // BUILDFLAG(IS_WIN)
  return data_dir;
}

base::FilePath GetUserSourceDataDir(const std::string& user_id,
                                    bool create_if_not_exists) {
  base::FilePath data_dir = GetDataDir();
  base::FilePath user_source_data_dir =
      data_dir.Append(FILE_PATH_LITERAL("user_data"));
  user_source_data_dir =
      user_source_data_dir.Append(path_unit::BaseFilePathFromU8(user_id));
  if (!base::PathExists(user_source_data_dir)) {
    bool result = false;
    if (create_if_not_exists) {
      base::File::Error err;
      result = base::CreateDirectoryAndGetError(user_source_data_dir, &err);
    }
    // 目录没创建成功,则返回空路径
    if (!result) {
      user_source_data_dir.clear();
    }
  }

  return user_source_data_dir;
}

base::FilePath CreateSubDir(base::FilePath parent,
                            base::FilePath::StringPieceType sub_name) {
  base::FilePath full_path = parent.Append(sub_name);
  if (!base::PathExists(full_path)) {
    base::File::Error err;
    bool result = base::CreateDirectoryAndGetError(full_path, &err);
    // 目录没创建成功,则返回空路径
    if (!result) {
      LOG(ERROR) << "create dir" << full_path << ", error:" << err;
      full_path.clear();
    }
  }

  return full_path;
}

base::FilePath GetPublicSourceDataDir() {
  base::FilePath data_dir = GetDataDir();
  base::FilePath public_source_data_dir =
      CreateSubDir(data_dir, FILE_PATH_LITERAL("user_data/public"));

  return public_source_data_dir;
}

base::FilePath GetGlobalConfigDir() {
  base::FilePath data_dir = GetDataDir();
  base::FilePath global_config_dir =
      CreateSubDir(data_dir, FILE_PATH_LITERAL("global_config"));

  return global_config_dir;
}

base::FilePath GetAppConfigDir() {
  base::FilePath data_dir = GetDataDir();
  base::FilePath app_config_dir =
      CreateSubDir(data_dir, FILE_PATH_LITERAL("app_config"));

  return app_config_dir;
}

int GetMinLogLevel(const base::FilePath& log_file_name) {
  int min_log_level = logging::LOG_INFO;

  base::FilePath current_version_entry =
      base::FilePath::FromUTF8Unsafe(GetNasEntryDir());
  DCHECK(!current_version_entry.value().empty());

  base::FilePath log_switch_dir =
      GetNasAppInstallDir().Append(current_version_entry);
  log_switch_dir = log_switch_dir.Append(FILE_PATH_LITERAL("services"));
  log_switch_dir = log_switch_dir.Append(FILE_PATH_LITERAL("log"));
  base::FilePath log_file_path = log_switch_dir.Append(log_file_name);
  base::FilePath log_info_flag =
      log_file_path.ReplaceExtension(FILE_PATH_LITERAL("log.info.on"));
  if (!base::PathExists(log_info_flag)) {
    min_log_level = logging::LOG_WARNING;
    base::FilePath log_warn_flag =
        log_file_path.ReplaceExtension(FILE_PATH_LITERAL("log.warning.on"));
    if (!base::PathExists(log_warn_flag)) {
      min_log_level = logging::LOG_ERROR;
      // ERROR级别以上的都必须开启,不再接受控制
      // base::FilePath log_error_flag =
      // log_file_path.ReplaceExtension(FILE_PATH_LITERAL("log.error.on"));
    }
  }

  return min_log_level;
}

void InitLog(const std::string& log_file,
             int log_min_level /*= logging::LOG_VERBOSE*/) {
  /* logging::SetLogMessageHandler([](int severity, const char* file, int line,
                                    size_t message_start,
                                    const std::string& str) { return true; });*/

  do {
    const uint32_t logging_dest = logging::LOG_TO_FILE |
                                  logging::LOG_TO_STDERR |
                                  logging::DELETE_OLD_LOG_FILE;
    logging::LoggingSettings settings;
    settings.logging_dest = logging_dest;

    auto log_file_path =
        GetNasLogDir().Append(base::FilePath::FromUTF8Unsafe(log_file));
    base::File::Error err;
    base::CreateDirectoryAndGetError(log_file_path.DirName(), &err);

    if (!log_file_path.empty()) {
      settings.log_file_path = log_file_path.value().c_str();
    }
    // To view log output with IDs and timestamps use "adb logcat -v
    // threadtime".
    logging::SetLogItems(true,    // Process ID
                         true,    // Thread ID
                         true,    // Timestamp
                         false);  // Tick count
    logging::InitLogging(settings);
    // 如果日志等级是默认值，则根据开关文件判断
    if (log_min_level == logging::LOG_VERBOSE) {
      log_min_level = GetMinLogLevel(base::FilePath::FromUTF8Unsafe(log_file));
    }
    logging::SetMinLogLevel(log_min_level);

  } while (false);
}

base::FilePath GetNasAppInstallDir() {
  base::FilePath app_install_dir;
  base::FilePath current_dir;
  base::FilePath last_dir;
  base::PathService::Get(base::DIR_EXE, &current_dir);
  int max_dir_level = kMaxDirDeepLevel;

  do {
    base::FilePath nas_json_path = current_dir.Append(kInstallEntryFile);
    if (base::PathExists(nas_json_path)) {
      app_install_dir = current_dir;
      LOG(INFO) << "install dir:" << app_install_dir;
      break;
    }
    current_dir = current_dir.DirName();
    max_dir_level--;
  } while (max_dir_level > 0 && current_dir.value().length() > 3);

  // last_dir = current_dir.DirName();

  //// 如果获取上一次目录等于当前目录,应该是到达根路径或者出错.返回
  // while (current_dir != last_dir) {
  //   current_dir = last_dir;
  //   base::FilePath nas_json_path = current_dir.Append(kInstallEntryFile);
  //   if(base::PathExists(nas_json_path)){
  //     app_install_dir = current_dir;
  //     LOG(INFO)<<"install dir:"<< app_install_dir;
  //     break;
  //   }
  //
  //   last_dir = current_dir.DirName();
  // }

  DCHECK(!app_install_dir.empty());
  if (app_install_dir.empty()) {
    LOG(ERROR) << " get install dir empty";
  }

  return app_install_dir;
}

base::FilePath GetNasWebResourceDir(int relative_level) {
  base::FilePath app_version_dir;
  base::PathService::Get(base::DIR_EXE, &app_version_dir);
  // 0 是当前目录层级
  while (relative_level--) {
    app_version_dir = app_version_dir.DirName();
  }
  return app_version_dir.Append(FILE_PATH_LITERAL("openresty"))
      .Append(FILE_PATH_LITERAL("html"));
}

base::FilePath GetWebDirectAccessDir(int relative_level) {
  return GetNasWebResourceDir(relative_level)
      .Append(FILE_PATH_LITERAL("public"));
}

base::FilePath GetNasConfigDir() {
  return GetNasHomeDir();
}

base::FilePath GetNasLogDir() {
  return NasHomePathAppend(FILE_PATH_LITERAL("log"));
}

base::FilePath GetNasDumpDir() {
  return NasHomePathAppend(FILE_PATH_LITERAL("dump"));
}

base::FilePath GetNasCacheDir() {
  base::FilePath data_dir = GetDataDir();
  base::FilePath cache_dir = CreateSubDir(data_dir, FILE_PATH_LITERAL("cache"));

  return cache_dir;
}

void ReadNasConf(unsigned short level, NasConf& nas_conf) {
  base::FilePath module_path;
  base::PathService::Get(base::DIR_EXE, &module_path);
  // 0 是当前目录层级
  while (level--) {
    module_path = module_path.DirName();
  }

  base::FilePath config_path =
      module_path.Append(base::FilePath::FromUTF8Unsafe("nas.json"));
  ReadNasConf(config_path, nas_conf);
}

base::FilePath GetPublicUserDataDir(const base::FilePath& kInstallDir) {
  return GetUserDataDir(kInstallDir).Append(FILE_PATH_LITERAL("public"));
}

base::FilePath GetPublicUserDataCacheDir(const base::FilePath& kInstallDir) {
  const base::FilePath kCacheDir =
      GetPublicUserDataDir(kInstallDir).Append(FILE_PATH_LITERAL(".cache"));
  if (!base::PathExists(kCacheDir)) {
    base::CreateDirectory(kCacheDir);
#if BUILDFLAG(IS_WIN)
    ::SetFileAttributes(kCacheDir.value().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
  }
  return kCacheDir;
}

base::FilePath GetNasJsonFilePath(unsigned short level) {
  base::FilePath module_path;
  base::PathService::Get(base::DIR_EXE, &module_path);
  // 0 是当前目录层级
  while (level--) {
    module_path = module_path.DirName();
  }

  base::FilePath config_path =
      module_path.Append(base::FilePath::FromUTF8Unsafe("nas.json"));
  return config_path;
}

base::FilePath Get7zFilePath(unsigned short level) {
  base::FilePath module_path;
  base::PathService::Get(base::DIR_EXE, &module_path);
  // 0 是当前目录层级
  while (level--) {
    module_path = module_path.DirName();
  }

#if BUILDFLAG(IS_WIN)
  module_path = module_path.Append(base::FilePath::FromUTF8Unsafe("7z.dll"));
#elif BUILDFLAG(IS_LINUX)
  module_path = module_path.Append(base::FilePath::FromUTF8Unsafe("7z.so"));
#endif
  return module_path;
}

base::FilePath Get7zaFilePath(unsigned short level) {
  base::FilePath module_path;
  base::PathService::Get(base::DIR_EXE, &module_path);
  // 0 是当前目录层级
  while (level--) {
    module_path = module_path.DirName();
  }

#if BUILDFLAG(IS_WIN)
  module_path = module_path.Append(base::FilePath::FromUTF8Unsafe("7za.exe"));
#elif BUILDFLAG(IS_LINUX)
  module_path = module_path.Append(base::FilePath::FromUTF8Unsafe("7za"));
#endif
  return module_path;
}


void ReadNasConf(const base::FilePath& config_path, NasConf& nas_conf) {
  if (!NasConfigGetString(config_path, "version", &nas_conf.version, "",
                          false)) {
    LOG(WARNING) << __func__ << " nas.json read version failed!";
  } else {
    // LOG(INFO) << __func__ << " nas_conf.version:" << nas_conf.version;
  }

  if (!NasConfigGetString(config_path, "entry_dir", &nas_conf.entry_dir, "",
                          false)) {
    LOG(WARNING) << __func__ << " nas.json read entry_dir failed!";
  } else {
    // LOG(INFO) << __func__ << " nas_conf.entry_dir:" << nas_conf.entry_dir;
  }

  if (!NasConfigGetString(config_path, "user_data_dir", &nas_conf.user_data_dir,
                          "", false)) {
    LOG(WARNING) << __func__ << " nas.json read user_data_dir failed!";
  } else {
    // LOG(INFO) << __func__
    //           << " nas_conf.user_data_dir:" << nas_conf.user_data_dir;
  }

  if (!NasConfigGetString(config_path, "sid", &nas_conf.sid, "", false)) {
    LOG(WARNING) << __func__ << " nas.json read sid failed!";
  } else {
    // LOG(INFO) << __func__ << " nas_conf.sid:" << nas_conf.sid;
  }
}

std::string ConvertNasEnvironment(const std::string& input,
                                  const std::string& user_id) {
  if (input.empty() || input.find("#") == std::string::npos) {
    return input;
  }

  // todo: 先简单处理（这样目前会更健壮点？）；若需要通用，可使用正则
  std::string result = input;

  if (result.find(R"(#homenasdata#)") != std::string::npos) {
    std::string replace_str = nas::GetDataDir().AsUTF8Unsafe();
    base::ReplaceChars(replace_str, "\\", "/", &replace_str);
    base::ReplaceSubstringsAfterOffset(&result, 0, R"(#homenasdata#)",
                                       replace_str);
  }

  if (result.find(R"(#homeuserdata#)") != std::string::npos) {
    std::string replace_str =
        nas::GetUserSourceDataDir("", false).AsUTF8Unsafe();
    base::ReplaceChars(replace_str, "\\", "/", &replace_str);
    base::ReplaceSubstringsAfterOffset(&result, 0, R"(#homeuserdata#)",
                                       replace_str);
  }

  if (result.find(R"(#userid#)") != std::string::npos) {
    base::ReplaceSubstringsAfterOffset(&result, 0, R"(#userid#)", user_id);
  }

  LOG(INFO) << ", result:" << result;

  return result;
}

bool IsAutoTestModel() {
  base::FilePath exe_dir;
  base::PathService::Get(base::DIR_EXE, &exe_dir);
  base::FilePath on_file = exe_dir.Append(FILE_PATH_LITERAL("auto_test_on"));
  return base::PathExists(on_file);
}

#if BUILDFLAG(IS_WIN)
std::string GetProfilePath(const std::string& sid) {
  std::string profile_path;
  std::wstring wsid = base::UTF8ToWide(sid);
  HKEY hKey;
  std::wstring profileListPath =
      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\" + wsid;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, profileListPath.c_str(), 0, KEY_READ,
                   &hKey) == ERROR_SUCCESS) {
    WCHAR profilePath[MAX_PATH];
    DWORD bufferSize = sizeof(profilePath);
    if (RegQueryValueEx(hKey, L"ProfileImagePath", nullptr, nullptr,
                        (LPBYTE)profilePath, &bufferSize) == ERROR_SUCCESS) {
      profile_path = base::WideToUTF8(profilePath);
    }
    RegCloseKey(hKey);
  }

  return profile_path;
}
#endif

}  // namespace nas
