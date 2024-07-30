/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-07-17 10:15:38
 */

#ifndef NAS_COMMON_GRPC_STUB_STORAGE_STUB_H
#define NAS_COMMON_GRPC_STUB_STORAGE_STUB_H

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/system_info/protos/storage.grpc.pb.h"
#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"

namespace nas {

class StorageServiceStub
    : public base::RefCountedThreadSafe<StorageServiceStub>,
      public GrpcStubBase {
 public:
  StorageServiceStub(HandlerRequestId handler_id,
                     const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;
 protected:
  virtual void AsyncInitStub(GetStubCallbackCallback) override;
  void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);

 private:
  std::unique_ptr<storage::v1::StorageService::Stub> storage_service_stub_;
  friend class base::RefCountedThreadSafe<StorageServiceStub>;
  virtual ~StorageServiceStub(){};
};  // StorageServiceStub

}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_STORAGE_STUB_H