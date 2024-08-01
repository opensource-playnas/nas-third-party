/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 19:32:19
 */

#include "network_task.h"
#include "parse_task.h"
#include "curl_export_define.h"
#include "curl_task_public_define.h"

namespace nas {

ParseTask::ParseTask() {
}

ParseTask::~ParseTask() {
}

int ParseTask::Parse(const char* url, unsigned long long* size) {
  Init(url);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(task_curl_ptr_, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_TIMEOUT, 10L);

  int retry = 2; // 最大重试次数
  CURLcode res;
  long response_code = 0;

  do {
    res = curl_easy_perform(task_curl_ptr_);
    if (res == CURLE_OK) {
      curl_off_t total_size = 0;
      curl_easy_getinfo(task_curl_ptr_, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &total_size);
      if (total_size < 0) {
        total_size = 0;
      }
      *size = total_size;

      curl_easy_getinfo(task_curl_ptr_, CURLINFO_RESPONSE_CODE, &response_code);

      if (total_size > 0 && response_code == 0) {
        return DownloadErrorCode::kNormal;
      }

      if (response_code >= HttpCode::kOk && response_code < HttpCode::kBadRequest) {
        break;
      } 
    }
    else {
      return (DownloadErrorCode)(res + DownloadErrorCode::kPerformCurlStartErrorCode);
    }
  } while (--retry);

  if (response_code < HttpCode::kOk || response_code >= HttpCode::kBadRequest) {
    DownloadErrorCode err = static_cast<DownloadErrorCode>(DownloadErrorCode::kHttpStartErrorCode + response_code);
    // 转换失败，错误码设置为kPerformCurlFailed
    if (err <= DownloadErrorCode::kHttpStartErrorCode || err > kHttpError504) {
      err = DownloadErrorCode::kPerformCurlFailed;
    }
    return err;
  }

  return DownloadErrorCode::kNormal;
}

void ParseTask::SetUrlOpt(CURL* task_curl) {
  if (!task_curl) {
    return;
  }
  curl_easy_setopt(task_curl, CURLOPT_HTTPGET, 1);
}

size_t ParseTask::HeaderCallback(void* buffer,
                       size_t size,
                       size_t nitems,
                       void* userdata) {
  const std::string header((char*)buffer, nitems * size);
  const std::string prefix = "Content-Length:";
  size_t pos = header.find(prefix);
  if (pos != std::string::npos) {
    size_t endpos = header.find("\r\n", pos + prefix.length());
    if (endpos != std::string::npos) {
      std::string length_str = header.substr(pos + prefix.length(),
                                             endpos - (pos + prefix.length()));
      *static_cast<double*>(userdata) = std::stod(length_str);
    }
  }
  return nitems * size;
}

}  // namespace nas
