/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-10-19 17:47:25
*/

#ifndef NAS_COMMON_GRPC_STUB_OPERATE_RECORDS_STUB_H_
#define NAS_COMMON_GRPC_STUB_OPERATE_RECORDS_STUB_H_

#include <map>
#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"

#include "nas/file_server/protos/file_server.grpc.pb.h"
#include "grpc_stub_base.h"
#include "grpc_hold_state_channel.h"
namespace nas {

class OperateRecordStub : public base::RefCountedThreadSafe<OperateRecordStub>,
                       public GrpcStubBase {
 public:
  OperateRecordStub(HandlerRequestId handler_id,
                 const std::multimap<std::string, std::string>& header);
  virtual bool AsyncHandler(const std::string& param,
                            AsyncGrpcResponseHandler grpc_response_handler) override;

 protected:
  virtual void AsyncInitStub(GetStubCallbackCallback) override;
  void Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler);
 private:
  // TaskRecordsService的stub 端
  std::unique_ptr<file::v1::TaskRecordsService::Stub>
      operate_records_service_stub_;
  friend class base::RefCountedThreadSafe<OperateRecordStub>;
  virtual ~OperateRecordStub(){};
};  // OperateRecordStub

}  // namespace nas
#endif // NAS_COMMON_GRPC_STUB_OPERATE_RECORDS_STUB_H_