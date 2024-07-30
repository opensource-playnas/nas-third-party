#include "system_info_stub.h"

#include "grpc_client_context.hpp"
#include "nas/common/micro_service_info.hpp"

namespace nas {
SystemInfoServiceStub::SystemInfoServiceStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kSystemInfoServiceName;
}

void SystemInfoServiceStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kHadrewareInfo:
      (new AsyncClientCall<system_info::v1::GetHardwareInfoResponse,
                           system_info::v1::GetHardwareInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::GetHardwareInfoRequest& request) {
             return system_info_service_stub_->PrepareAsyncGetHardwareInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kNetInfo:
      (new AsyncClientCall<system_info::v1::GetNetInfoResponse,
                           system_info::v1::GetNetInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::GetNetInfoRequest& request) {
             return system_info_service_stub_->PrepareAsyncGetNetInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kGetBaseStaticInfo:
      (new AsyncClientCall<system_info::v1::GetBaseStaticInfoResponse,
                           system_info::v1::GetBaseStaticsInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::GetBaseStaticsInfoRequest& request) {
             return system_info_service_stub_->PrepareAsyncGetBaseStaticInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kSetBaseStaticInfo:
      (new AsyncClientCall<system_info::v1::SetBaseStaticInfoResponse,
                           system_info::v1::SetBaseStaticsInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::SetBaseStaticsInfoRequest& request) {
             return system_info_service_stub_->PrepareAsyncSetBaseStaticInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kGetTimeStaticInfo:
      (new AsyncClientCall<system_info::v1::GetTimeStaticsInfoResponse,
                           system_info::v1::GetTimeStaticsInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::GetTimeStaticsInfoRequest& request) {
             return system_info_service_stub_->PrepareAsyncGetTimeStaticInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetCacheInfo:
      (new AsyncClientCall<system_info::v1::GetCacheInfoResponse,
                           system_info::v1::GetCacheInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::GetCacheInfoRequest& request) {
             return system_info_service_stub_->PrepareAsyncGetCacheInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kCleanCache:
      (new AsyncClientCall<system_info::v1::CleanCacheResponse,
                           system_info::v1::CleanCacheRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::CleanCacheRequest& request) {
             return system_info_service_stub_->PrepareAsyncCleanCache(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kShutdownOs:
      (new AsyncClientCall<system_info::v1::ShutdownNativeOsResponse,
                           system_info::v1::ShutdownNativeOsRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::ShutdownNativeOsRequest& request) {
             return system_info_service_stub_->PrepareAsyncShutdownNativeOs(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kRestartOS:
      (new AsyncClientCall<system_info::v1::RestartNativeOsResponse,
                           system_info::v1::RestartNativeOsRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::RestartNativeOsRequest& request) {
             return system_info_service_stub_->PrepareAsyncRestartNativeOs(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kFeedback:
      (new AsyncClientCall<system_info::v1::UploadFeedBackResponse,
                           system_info::v1::UploadFeedBackRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const system_info::v1::UploadFeedBackRequest& request) {
             return system_info_service_stub_->PrepareAsyncUploadFeedBack(
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

bool SystemInfoServiceStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
 
  AsyncInitStub(base::BindOnce(&SystemInfoServiceStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void SystemInfoServiceStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](SystemInfoServiceStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->system_info_service_stub_ =
      system_info::v1::SystemInfoService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas