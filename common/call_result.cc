/*
 * @Description: nas 接口调用结果封装
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2022-08-31 18:49:38
 */

#include "call_result.h"

#include "base/base64.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/check.h"

namespace nas {
ErrorSource CallResult::error_source = ErrorSource::kOk;
WrapperSetResult::WrapperSetResult(grpc::ServerContext* context)
    : context_(context) {}

void WrapperSetResult::SetResult(const CallResult& call_result) {
  if (context_) {
    ErrorHandle error_handle(call_result.error_source);
    error_handle.SetErrorNo(call_result.errcode);
    error_handle.SetErrorMsg(call_result.msg);


    context_->AddInitialMetadata("errno", std::to_string(error_handle.GetErrorNo()));
    
    std::string msg_base64;
    base::Base64Encode(call_result.msg, &msg_base64);
    // msg里面带有中文会导致crash,所以base64,见该函数头文件
    context_->AddInitialMetadata("msg-bin", msg_base64);
  }
}

void WrapperSetResult::AddMateData(const std::string& key,
                                   const std::string& value) {
  if (!context_) {
    return;
  }
  // 需要把key值转为小写
  auto lower_key = base::ToLowerASCII(key);
  context_->AddInitialMetadata(lower_key, value);
}

int GetTransErrorNo(uint32_t err) {
    ErrorHandle error_handle(nas::CallResult::error_source);
    error_handle.SetErrorNo(err);
    return error_handle.GetErrorNo();
}

};  // namespace nas
