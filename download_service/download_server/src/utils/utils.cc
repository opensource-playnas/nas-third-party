
/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:51:13
 */
#include "utils.h"

#include <iostream>
#include <regex>
#include <string>

#include "base/logging.h"
#include "base/strings/escape.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"

#include "download_error_desc.h"
#include "nas/common/nas_config.h"
#include "nas/common/path/file_path_unit.h"

namespace nas {
namespace utils {

nas::CallResult ErrNoToCallResult(DownloadErrorCode err_no) {
  uint32_t err_code = static_cast<uint32_t>(err_no);
  std::string msg = nas::GetDownloadErrorDesc(err_no);
  return nas::CallResult{err_code, msg};
}

void SetRpcResult(DownloadErrorCode err_no, nas::WrapperSetResult* rpc_result) {
  if (!rpc_result) {
    return;
  }
  auto call_result = utils::ErrNoToCallResult(err_no);
  if (err_no != DownloadErrorCode::kNormal) {
    LOG(WARNING) << "SetRpcResult, err: " << call_result.errcode
                 << ", msg: " << call_result.msg;
  }

  rpc_result->SetResult(std::move(call_result));
}

bool IsValid(std::string& url) {
  bool ret = false;
  do {
    if (url.empty() || url[0] == '\0') {
      LOG(ERROR) << "add download url is null, so failed";
      break;
    }

    GURL gurl(url);
    ret = gurl.is_valid();
    if (!ret) {
      LOG(ERROR) << "add download url is invalid";
      break;
    }

    // 如果path中是urlencode, 需要对path进行urldecode
    std::string path = gurl.path();
    std::string decoded_path =
        base::UnescapeURLComponent(path, base::UnescapeRule::NORMAL);
    if (path == decoded_path) {
      break;
    }
    std::size_t path_start = url.find(path);
    if (path_start != std::string::npos) {
      url = url.substr(0, path_start) + decoded_path +
            url.substr(path_start + path.length());
    }

    GURL new_gurl(url);
    ret = new_gurl.is_valid();
    if (!ret) {
      LOG(ERROR) << "add download url is invalid";
      break;
    }
  } while (0);

  return ret;
}

time_t GetCurrnetTime() {
  base::Time time = base::Time::Now();
  time_t current_time = time.ToTimeT();
  return current_time;
}

base::FilePath GetTorrentFileSavePath() {
  base::FilePath dir_path = GetDownloadConfigDir();
  dir_path = dir_path.Append(FILE_PATH_LITERAL("torrent_file"));
  base::File::Error err = base::File::FILE_OK;
  if (!base::CreateDirectoryAndGetError(dir_path, &err)) {
    base::FilePath temp_path;
    base::GetTempDir(&temp_path);
    return temp_path;
  }

  return dir_path;
}

base::FilePath GetUploadTorrentFileCachePath() {
  base::FilePath dir_path = nas::GetNasCacheDir();
  dir_path = dir_path.Append(FILE_PATH_LITERAL("torrent_file"));
  base::File::Error err = base::File::FILE_OK;
  if (!base::CreateDirectoryAndGetError(dir_path, &err)) {
    base::FilePath temp_path;
    base::GetTempDir(&temp_path);
    return temp_path;
  }

  return dir_path;
}

base::FilePath GetDownloadConfigDir() {
  base::FilePath config_dir = nas::GetAppConfigDir();
  config_dir = config_dir.Append(FILE_PATH_LITERAL("download_service"));
  return config_dir;
}

std::string GetSuffixByName(const std::string& file_name) {
  base::FilePath path;
#if BUILDFLAG(IS_WIN)
  path = path_unit::BaseFilePathFromU8(file_name);
  base::FilePath::StringType suffix = path.BaseName().FinalExtension();
  return base::WideToUTF8(suffix);
#elif BUILDFLAG(IS_POSIX)
  path = base::FilePath(file_name);
  base::FilePath::StringType suffix = path.BaseName().FinalExtension();
  return suffix;
#endif
}

int DownloadErrNoToOutErrNo(DownloadErrorCode err_no) {
  return nas::GetTransErrorNo(err_no);
}

void Notify(const std::string& operate_id,
            base::Value::Dict msg,
            const ContextField& context_field) {
  ContextField context(context_field.nas_token(), context_field.user_id(),
                       operate_id);
  std::string content_json;
  base::JSONWriter::Write(msg, &content_json);
}

int64_t StringToInt64(const std::string& str) {
  int64_t result = 0;
  if (base::StringToInt64(str, &result)) {
    return result;
  }
  return 0;
}

int StringToInt(const std::string& str) {
  int result = 0;
  if (base::StringToInt(str, &result)) {
    return result;
  }
  return 0;
}

bool DeleteFile(const std::string& path) {
  base::FilePath file_path = nas::path_unit::BaseFilePathFromU8(path);
  if (base::PathExists(file_path) && !base::DeleteFile(file_path)) {
    LOG(WARNING) << "delete torrent file failed, path :" << path;
    return false;
  }
  return true;
}

bool IsValidFilename(const std::string& file_name) {
  // Windows文件名命名规则：
  // 文件名不允许包含以下字符：<、>、:、"、/、\、|、?、*。
  // 文件名不允许以空格或点（.）结尾。
  // Windows 保留了一些特殊的文件名，如：CON, PRN, AUX, NUL, COM1, COM2, COM3,
  // COM4, COM5, COM6, COM7, COM8, COM9, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6,
  // LPT7, LPT8, LPT9。
  // 不能是回车换行

  // Linux文件名命名规则：
  // 文件名不允许包含 / 和 \0（空字符）。
  // Linux
  // 对文件名没有其他特殊限制，但通常建议避免使用特殊字符，如空格、制表符、换行符等

#if BUILDFLAG(IS_WIN)
  // 最后一个字符不能是空格或者.
  if (file_name.back() == ' ' || file_name.back() == '.') {
    return false;
  }

  // Regular expression for invalid characters and reserved filenames on Windows
  std::regex rule(R"([<>:"/\\|?*\x09\x0A\x0D]|(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(\..*|$))",
                  std::regex_constants::icase);

#elif BUILDFLAG(IS_POSIX)
  // Regular expression for invalid characters on Linux
  std::regex rule(R"([/\x00\x20\x09\x0A])");
#endif
  if (std::regex_search(file_name, rule)) {
    return false;
  }
  return true;
}

}  // namespace utils
}  // namespace nas
