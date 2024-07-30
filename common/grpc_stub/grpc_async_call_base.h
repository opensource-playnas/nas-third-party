
/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-07-31 13:17:55
*/

#include <memory>
#include "grpcpp/impl/codegen/status.h"

#ifndef NAS_COMMON_GRPC_STUB_GRPC_ASYNC_CALL_BASH_H_
#define NAS_COMMON_GRPC_STUB_GRPC_ASYNC_CALL_BASH_H_
namespace nas {

class GrpcClientContext;
class AsyncClientCallBase {
public:
  AsyncClientCallBase(std::shared_ptr<GrpcClientContext> context){
    context_ = context;
  }
  virtual ~AsyncClientCallBase() {}
  virtual void HandleResponse() = 0;
  std::shared_ptr<GrpcClientContext> context_;
  grpc::Status status;
};
    
} // nas
#endif // NAS_GATEWAY_GRPC_RESOURCE_GRPC_ASYNC_CALL_BASH_H_