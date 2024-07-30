// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-07-17 18:01:27

#ifndef NAS_COMMON_GRPC_STUB_PICTURE_SERVICE_STUB_H_
#define NAS_COMMON_GRPC_STUB_PICTURE_SERVICE_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/picture_server/protos/picture_service.grpc.pb.h"
#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"

namespace nas {

class PictureServiceStub
    : public base::RefCountedThreadSafe<PictureServiceStub>,
      public GrpcStubBase {
 public:
  PictureServiceStub(HandlerRequestId handler_id,
                     const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;

 protected:
  virtual void AsyncInitStub(GetStubCallbackCallback) override;
  void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);

 private:
  std::unique_ptr<picture::v1::PictureService::Stub> picture_service_stub_;
  friend class base::RefCountedThreadSafe<PictureServiceStub>;
  ~PictureServiceStub(){};
};

}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_PICTURE_SERVICE_STUB_H_