/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-10 13:47:55
 */

#include "dlna_service_stub.h"

#include "google/protobuf/util/json_util.h"
#include "nas/common/micro_service_info.hpp"
#include "grpc_client_context.hpp"

namespace nas {
DlnaServerStub::DlnaServerStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kDlnaService;
}


void DlnaServerStub::Dispatch(const std::string& param,
                               AsyncGrpcResponseHandler grpc_response_handler) {

  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;
  bool processed = true;
  switch (handler_id_) {    
    case HandlerRequestId::kListDlnaDevice:
      (new AsyncClientCall<dlna::v1::ListScreenCastingDevicesResponse,
                           dlna::v1::ListScreenCastingDevicesRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::ListScreenCastingDevicesRequest& request) {
             return dlna_service_stub_->PrepareAsyncListScreenCastingDevices(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;      
    case HandlerRequestId::kCreateDlnaTask:
      (new AsyncClientCall<dlna::v1::CreateScreenCastingTaskResponse,
                           dlna::v1::CreateScreenCastingTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::CreateScreenCastingTaskRequest& request) {
             return dlna_service_stub_->PrepareAsyncCreateScreenCastingTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;        
    case HandlerRequestId::kCancelDlnaTask:
      (new AsyncClientCall<dlna::v1::CancelScreenCastingTaskResponse,
                           dlna::v1::CancelScreenCastingTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::CancelScreenCastingTaskRequest& request) {
             return dlna_service_stub_->PrepareAsyncCancelScreenCastingTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;         
    case HandlerRequestId::kChangeDlnaTaskPlayUrl:
      (new AsyncClientCall<dlna::v1::ChangeScreenCastingTaskPlayUrlResponse,
                           dlna::v1::ChangeScreenCastingTaskPlayUrlRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::ChangeScreenCastingTaskPlayUrlRequest& request) {
             return dlna_service_stub_->PrepareAsyncChangeScreenCastingTaskPlayUrl(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;         
    case HandlerRequestId::kPlayDlnaTask:
      (new AsyncClientCall<dlna::v1::PlayScreenCastingTaskResponse,
                           dlna::v1::PlayScreenCastingTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::PlayScreenCastingTaskRequest& request) {
             return dlna_service_stub_->PrepareAsyncPlayScreenCastingTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;          
    case HandlerRequestId::kPauseDlnaTask:
      (new AsyncClientCall<dlna::v1::PauseScreenCastingTaskResponse,
                           dlna::v1::PauseScreenCastingTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::PauseScreenCastingTaskRequest& request) {
             return dlna_service_stub_->PrepareAsyncPauseScreenCastingTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;           
    case HandlerRequestId::kSeekDlnaTask:
      (new AsyncClientCall<dlna::v1::SeekScreenCastingTaskResponse,
                           dlna::v1::SeekScreenCastingTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::SeekScreenCastingTaskRequest& request) {
             return dlna_service_stub_->PrepareAsyncSeekScreenCastingTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;         
    case HandlerRequestId::kSetDlnaTaskVolume:
      (new AsyncClientCall<dlna::v1::SetScreenCastingTaskVolumeResponse,
                           dlna::v1::SetScreenCastingTaskVolumeRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const dlna::v1::SetScreenCastingTaskVolumeRequest& request) {
             return dlna_service_stub_->PrepareAsyncSetScreenCastingTaskVolume(
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
bool DlnaServerStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);

  AsyncInitStub(base::BindOnce(&DlnaServerStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void DlnaServerStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](DlnaServerStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->dlna_service_stub_ =
                dlna::v1::DlnaService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}

}  // namespace nas