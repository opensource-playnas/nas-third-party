/*
 * @Description: nas 接口调用结果封装
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2022-08-31 18:49:38
 */

#ifndef NAS_COMMON_CALL_RESULT_H_
#define NAS_COMMON_CALL_RESULT_H_

#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/server_context.h>

#include "nas/common/error_code/general_error_code.h"

namespace nas {

struct CallResult {
  // 错误源,每个独立模块必须按文档定义自己的值
  static ErrorSource error_source;
  uint32_t errcode = 0;         // 错误码
  std::string msg = "success";  // 错误信息
};

class WrapperSetResult {
 public:
   explicit WrapperSetResult(grpc::ServerContext* context);
  ~WrapperSetResult() = default;

  void SetResult(const CallResult& call_result);
  void AddMateData(const std::string& key,const std::string& value);
 private:
  grpc::ServerContext* context_;
};

int GetTransErrorNo(uint32_t err);

};  // namespace nas

#endif  // NAS_COMMON_CALL_RESULT_H_
