/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:32:11
 */

#ifndef NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_TASK_PUBLIC_DEFINE_H_
#define NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_TASK_PUBLIC_DEFINE_H_

#include "download_public_define.h"

class CurlDownloadTask : public DownloadTaskInterface {
public:
  virtual ~CurlDownloadTask() = default;
  virtual void SetParam(const DownloadParam& param) = 0;
  virtual int ParseLink(const char* link, ParseResult* result) = 0;
  virtual bool ResumeBreakPoint() = 0;
  virtual void SetMaxDownloadSpeed(int64_t max_download_speed) = 0;
  virtual void Reset() = 0;
  virtual void TryReStart() = 0;
};

#endif  // NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_TASK_PUBLIC_DEFINE_H_
