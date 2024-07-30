/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-10 13:47:55
 */

#include "media_service_stub.h"

#include "google/protobuf/util/json_util.h"
#include "nas/common/micro_service_info.hpp"
#include "grpc_client_context.hpp"

namespace nas {
MediaServerStub::MediaServerStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kMediaService;
}


void MediaServerStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kVideoInfo:
      (new AsyncClientCall<media::v1::GetMediaInfoResponse,
                           media::v1::GetMediaInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::GetMediaInfoRequest& request) {
             return media_service_stub_->PrepareAsyncGetMediaInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kVideoPlay:
      (new AsyncClientCall<media::v1::StartPlayResponse,
                           media::v1::StartPlayRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::StartPlayRequest& request) {
             return media_service_stub_->PrepareAsyncStartPlay(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVideoPlayByFile:
      (new AsyncClientCall<media::v1::StartPlayByFileResponse,
                           media::v1::StartPlayByFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::StartPlayByFileRequest& request) {
             return media_service_stub_->PrepareAsyncStartPlayByFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVideoPlayTs:
      (new AsyncClientCall<media::v1::GetPlayTsResponse,
                           media::v1::GetPlayTsRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::GetPlayTsRequest& request) {
             return media_service_stub_->PrepareAsyncGetPlayTs(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVideoSubtitle:
      (new AsyncClientCall<media::v1::GetSubtitleResponse,
                           media::v1::GetSubtitleRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::GetSubtitleRequest& request) {
             return media_service_stub_->PrepareAsyncGetSubtitle(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVideoPlayTransCode:
      (new AsyncClientCall<media::v1::QueryTranscodeInfoResponse,
                           media::v1::QueryTranscodeInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::QueryTranscodeInfoRequest& request) {
             return media_service_stub_->PrepareAsyncQueryTranscodeInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVideoQueryPlayTaskInfo:
      (new AsyncClientCall<media::v1::QueryPlayTaskInfoResponse,
                           media::v1::QueryPlayTaskInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::QueryPlayTaskInfoRequest& request) {
             return media_service_stub_->PrepareAsyncQueryPlayTaskInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVideoPlayCancel:
      (new AsyncClientCall<media::v1::CancelPlayResponse,
                           media::v1::CancelPlayRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const media::v1::CancelPlayRequest& request) {
             return media_service_stub_->PrepareAsyncCancelPlay(
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
       

bool MediaServerStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  
  AsyncInitStub(base::BindOnce(&MediaServerStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void MediaServerStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](MediaServerStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->media_service_stub_ =
                media::v1::MediaService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas