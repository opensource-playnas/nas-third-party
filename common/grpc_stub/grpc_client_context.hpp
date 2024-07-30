/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-02 19:42:19
 */
#ifndef NAS_COMMON_GRPC_STUB_GRPC_CLIENT_CONTEXT_HPP_
#define NAS_COMMON_GRPC_STUB_GRPC_CLIENT_CONTEXT_HPP_

#include <map>
#include <string>

#include "base/base64.h"
#include "base/strings/string_util.h"

#include "grpcpp/client_context.h"
#include "grpc/slice.h"

#include "nas/common/error_code/general_error_code.h"

namespace nas {
class GrpcClientContext {
 public:
  grpc::ClientContext& ClientContext() { return client_context_; }
  
  int GetErrorNo() {
    for (auto& iter : client_context_.GetServerInitialMetadata()) {
      std::string key(iter.first.data());
      std::string value(iter.second.data());
      if (iter.first == "errno") {
        return atoi(iter.second.data());
      }
    }

    return -100;
  }

  std::string GetErrorMsg() {
    std::string msg;
    bool exist_key = false;
    for (auto& iter : client_context_.GetServerInitialMetadata()) {
      std::string key(iter.first.data(),iter.first.length());
      std::string value(iter.second.data(),iter.second.length());
      if (key == "msg-bin") {
        exist_key = true;
        std::string de_value;
        if (base::Base64Decode(value, &de_value)) {
          msg = de_value;
          break;
        }
      }
    }

    if(!exist_key){
      msg = "service unavailable";
    }

    return msg;
  }

  std::multimap<std::string, std::string> GrpcMetaToHttpHeader(){ 
    std::multimap<std::string, std::string> http_response_header;
    for(const auto& iter : client_context_.GetServerInitialMetadata()){
      // 过滤掉errno和msg单独处理
      if(!base::CompareCaseInsensitiveASCII(iter.first.data(), "errno") && !base::CompareCaseInsensitiveASCII(iter.first.data(), "msg-bin")){
        http_response_header.insert(std::make_pair(iter.first.data(), iter.second.data()));
      }
    }
    return http_response_header;
  }

 private:
  grpc::ClientContext client_context_;
};  // class GrpcClientContext

}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_GRPC_CLIENT_CONTEXT_HPP_