/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-03-06 19:51:02
*/

#ifndef NAS_DOWNLOAD_SERVER_SRC_UTILS_UTILS_H_
#define NAS_DOWNLOAD_SERVER_SRC_UTILS_UTILS_H_

#include "base/time/time.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "nas/common/call_result.h"
#include "nas/download_service/third_party/download_public_define.h"
#include "task/task_common_define.h"

namespace nas {

namespace utils {

nas::CallResult ErrNoToCallResult(DownloadErrorCode err_no);

void SetRpcResult(DownloadErrorCode err_no,
                  nas::WrapperSetResult* rpc_result);

bool IsValid(std::string& url);

time_t GetCurrnetTime();

base::FilePath GetTorrentFileSavePath();
base::FilePath GetUploadTorrentFileCachePath();

// 下载服务配置目录
base::FilePath GetDownloadConfigDir();

std::string GetSuffixByName(const std::string& file_name);

// 将下载错误码转成对外输出错误码
int DownloadErrNoToOutErrNo(DownloadErrorCode err_no);

void Notify(const std::string& operate_id,
                             base::Value::Dict msg,
                             const ContextField& context_field);
int64_t StringToInt64(const std::string& str);
int StringToInt(const std::string& str);
bool DeleteFile(const std::string& path);
bool IsValidFilename(const std::string& file_name);
}  // namespace utils
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_UTILS_UTILS_H_