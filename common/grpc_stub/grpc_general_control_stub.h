/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-09-21 20:17:18
 */

#ifndef NAS_COMMON_GRPC_STUB_GRPC_GENERAL_CONTROL_STUB_H_
#define NAS_COMMON_GRPC_STUB_GRPC_GENERAL_CONTROL_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "grpc_stub_base.h"
#include "nas/common/grpc_base_service/protos/grpc_base_service.grpc.pb.h"
// 控制grpc 一些常用的stub
namespace nas {
class GrpcGeneralControlStub
    : public base::RefCountedThreadSafe<GrpcGeneralControlStub>,
      public GrpcStubBase {
 public:
  GrpcGeneralControlStub(HandlerRequestId handler_id,
                         const std::multimap<std::string, std::string>& header,
                         const std::string& service_name);

  virtual bool AsyncHandler(
      const std::string& param,
      AsyncGrpcResponseHandler grpc_response_handler) override;

 protected:
   virtual void AsyncInitStub(GetStubCallbackCallback) override;
   void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);

  // attribute
 private:
  // grpc base的stub 端
  std::unique_ptr<GrpcBaseService::Stub> grpc_base_stub_;
  friend class base::RefCountedThreadSafe<GrpcGeneralControlStub>;
  virtual ~GrpcGeneralControlStub(){};
};  // class GrpcGeneralControlStub
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_GRPC_GENERAL_CONTROL_STUB_H_