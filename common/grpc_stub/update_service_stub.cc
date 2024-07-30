// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/04/25 13:31

#include "update_service_stub.h"

#include "google/protobuf/util/json_util.h"
#include "nas/common/micro_service_info.hpp"

#include "grpc_client_context.hpp"

namespace nas {

UpdateServerStub::UpdateServerStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kUpdateServiceName;
}

void UpdateServerStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kQueryUpdateInfo:
      (new AsyncClientCall<update_service::v1::UpdateInfoResponse,
                           update_service::v1::UpdateInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const update_service::v1::UpdateInfoRequest& request) {
             return update_service_stub_->PrepareAsyncQueryUpdateInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kDownloadUpdatePackage:
      (new AsyncClientCall<update_service::v1::OpResult,
                           update_service::v1::DownloadUpdatePackageRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const update_service::v1::DownloadUpdatePackageRequest& request) {
             return update_service_stub_->PrepareAsyncDownloadUpdatePackage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kCancelDownloadUpdatePackage:
      (new AsyncClientCall<update_service::v1::OpResult,
                           update_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const update_service::v1::EmptyParam& request) {
             return update_service_stub_->PrepareAsyncCancelDownloadUpdatePackage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kInstallUpdatePackage:
      (new AsyncClientCall<update_service::v1::OpResult,
                           update_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const update_service::v1::EmptyParam& request) {
             return update_service_stub_->PrepareAsyncInstallUpdatePackage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kCheckInstallResult:
      (new AsyncClientCall<update_service::v1::UpdateResultResponse,
                           update_service::v1::UpdateResultRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const update_service::v1::UpdateResultRequest& request) {
             return update_service_stub_->PrepareAsyncCheckUpdateResult(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    default: {
      processed = false;
      LOG(ERROR) << "error branch";
    }
  }
  DCHECK(processed);
}

bool UpdateServerStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  AsyncInitStub(base::BindOnce(&UpdateServerStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void UpdateServerStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](UpdateServerStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->update_service_stub_ = update_service::v1::UpdateService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas