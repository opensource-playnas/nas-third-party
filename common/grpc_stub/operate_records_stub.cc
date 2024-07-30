/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-10-19 17:47:29
*/

#include "operate_records_stub.h"

#include "google/protobuf/util/json_util.h"
#include "nas/common/micro_service_info.hpp"

#include "grpc_client_context.hpp"

namespace nas {
OperateRecordStub::OperateRecordStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kFileServiceName;
}

void OperateRecordStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;
  bool processed = true;
  switch (handler_id_) {   
    case HandlerRequestId::kListTaskRecords:
      (new AsyncClientCall<file::v1::ListTaskRecordsResponse,
                           file::v1::ListTaskRecordsRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::ListTaskRecordsRequest& request) {
             return operate_records_service_stub_->PrepareAsyncListTaskRecords(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kDeleteTaskRecords:
      (new AsyncClientCall<file::v1::DeleteTaskRecordsResponse,
                           file::v1::DeleteTaskRecordsRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::DeleteTaskRecordsRequest& request) {
             return operate_records_service_stub_->PrepareAsyncDeleteTaskRecords(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    default:
      processed = false;
      break;
  }
  DCHECK(processed);
}
bool OperateRecordStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);

  AsyncInitStub(base::BindOnce(&OperateRecordStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void OperateRecordStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](OperateRecordStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->operate_records_service_stub_ =
                file::v1::TaskRecordsService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas
