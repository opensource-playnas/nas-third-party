/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-12-26 19:40:21
*/
#ifndef NAS_COMMON_GRPC_STUB_GRPC_HEALTH_CHECK_STUB_H_
#define NAS_COMMON_GRPC_STUB_GRPC_HEALTH_CHECK_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "third_party/grpc/src/src/proto/grpc/health/v1/health.grpc.pb.h"

#include "grpc_stub_base.h"

// 健康检查
namespace nas {
class GrpcHealthCheckStub
    : public base::RefCountedThreadSafe<GrpcHealthCheckStub>,
      public GrpcStubBase {
 public:
  GrpcHealthCheckStub(HandlerRequestId handler_id,
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
  // grpc health check的stub 端
  std::unique_ptr<grpc::health::v1::Health::Stub> hc_stub_;
  friend class base::RefCountedThreadSafe<GrpcHealthCheckStub>;
  virtual ~GrpcHealthCheckStub(){};
};  // class GrpcHealthCheckStub
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_GRPC_HEALTH_CHECK_STUB_H_