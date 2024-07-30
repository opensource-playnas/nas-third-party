/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-05-30 17:42:57
 */

#ifndef NAS_COMMON_GRPC_STUB_SYSTEM_INFO_STUB_H
#define NAS_COMMON_GRPC_STUB_SYSTEM_INFO_STUB_H

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/system_info/protos/system_info.grpc.pb.h"
#include "nas/system_info/protos/storage.grpc.pb.h"

#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"

namespace nas {

class SystemInfoServiceStub
    : public base::RefCountedThreadSafe<SystemInfoServiceStub>,
      public GrpcStubBase {
 public:
  SystemInfoServiceStub(HandlerRequestId handler_id,
                        const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;
  
 protected:
  virtual void AsyncInitStub(GetStubCallbackCallback) override;
  void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);

 private:
  std::unique_ptr<system_info::v1::SystemInfoService::Stub>
      system_info_service_stub_;
  friend class base::RefCountedThreadSafe<SystemInfoServiceStub>;
  virtual ~SystemInfoServiceStub(){};

}; // SystemInfoServiceStub

}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_SYSTEM_INFO_STUB_H