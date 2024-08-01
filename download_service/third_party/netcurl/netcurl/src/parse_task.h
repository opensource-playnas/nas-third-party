/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 19:32:19
 */


#ifndef NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_PARSE_TASK_H_
#define NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_PARSE_TASK_H_

#include "network_task.h"

namespace nas {

class ParseTask : public NetworkTask{
 public:
  ParseTask();
  virtual ~ParseTask();
  // 返回0代表成功，非0未失败
  virtual int Parse(const char * url, unsigned long long* size);
  virtual Type TaskType() { return Type::kHttpGet; }
  virtual void SetUrlOpt(CURL* task_curl);
  static size_t HeaderCallback(void* buffer, size_t size, size_t nitems, void* userdata);
};
}  // namespace nas

#endif  // NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_PARSE_TASK_H_