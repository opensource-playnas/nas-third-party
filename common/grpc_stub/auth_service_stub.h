/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 15:16:48
 */

#ifndef NAS_COMMON_GRPC_STUB_AUTH_SERVICE_STUB_H_
#define NAS_COMMON_GRPC_STUB_AUTH_SERVICE_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "grpc_stub_base.h"
#include "nas/auth_service/protos/auth_service.grpc.pb.h"

namespace nas {
class AuthServiceStub : public base::RefCountedThreadSafe<AuthServiceStub>,
                        public GrpcStubBase {
 public:
  AuthServiceStub(HandlerRequestId handler_id,
                  const std::multimap<std::string, std::string>& header);

  virtual bool AsyncHandler(
      const std::string& param,
      AsyncGrpcResponseHandler grpc_response_handler) override;

 protected:
  virtual void AsyncInitStub(GetStubCallbackCallback) override;
  void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);
  // attribute
 private:
  // 鉴权服务器的stub 端
  std::unique_ptr<auth_service::v1::AuthService::Stub> auth_service_stub_;
  friend class base::RefCountedThreadSafe<AuthServiceStub>;
  virtual ~AuthServiceStub(){};

};  // class AuthServiceStub
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_AUTH_SERVICE_STUB_H_