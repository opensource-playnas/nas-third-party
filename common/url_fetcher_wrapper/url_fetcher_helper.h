/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-21 10:27:35
 */

#ifndef NAS_COMMON_URL_FETCHER_H_
#define NAS_COMMON_URL_FETCHER_H_

#include <map>
#include <memory>
#include <string>

#include "base/time/time.h"
#include "base/timer/timer.h"

#include "base/json/json_reader.h"

namespace nas {
class HttpResponseData {
  public:
    HttpResponseData(const std::string& response);
    bool IsValid();
    bool GetData(base::Value** value);
    bool GetErrorNo(int* error_no);
    bool GetErrorMessage(std::string* msg);


  private:
    // 存储parse后的json对象
    absl::optional<base::Value> json_obj_;
    base::Value* data_;
    // 错误码
    int error_no_;
    // 错误消息
    std::string msg_;

};


class UrlFetcherHelper {
 public:
  // 同步拉取配置
  static bool SyncGetString(const std::string& url,
                            std::map<std::string, std::string> header,
                            base::TimeDelta delay,
                            std::string* response);

  static bool SyncPostString(const std::string& url,
                             const std::string& data,
                             std::map<std::string, std::string> header,
                             base::TimeDelta delay,
                             std::string* response, int* response_code);

};  // class UrlFetcherHelper

}  // namespace nas

#endif  // NAS_COMMON_URL_FETCHER_H_