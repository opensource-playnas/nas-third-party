// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/04/25 13:30


#ifndef NAS_COMMON_GRPC_STUB_UPDATE_SERVICE_STUB_H_
#define NAS_COMMON_GRPC_STUB_UPDATE_SERVICE_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/update/protos/update_service.grpc.pb.h"

#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"
namespace nas {

class UpdateServerStub : public base::RefCountedThreadSafe<UpdateServerStub>,
                       public GrpcStubBase {
                        
 public:
  UpdateServerStub(HandlerRequestId handler_id,
                  const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;
 protected:  
  virtual void AsyncInitStub(GetStubCallbackCallback) override;
  void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);
 private:
  // stub 端
  std::unique_ptr<update_service::v1::UpdateService::Stub> update_service_stub_;
  friend class base::RefCountedThreadSafe<UpdateServerStub>;
  virtual ~UpdateServerStub(){};
                       
}; // MediaServerStub

}  // namespace nas
#endif  // NAS_COMMON_GRPC_STUB_UPDATE_SERVICE_STUB_H_