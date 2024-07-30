/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-17 11:02:41
 */

#include "download_service_stub.h"
#include "google/protobuf/util/json_util.h"
#include "nas/common/micro_service_info.hpp"

#include "grpc_client_context.hpp"

namespace nas {
DownloadServerStub::DownloadServerStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kDownloadServiceName;
}

void DownloadServerStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;
  bool processed = true;
  switch (handler_id_) {    
    case HandlerRequestId::kCreateLinkDownloadTask:
      (new AsyncClientCall<download::v1::CreateLinkDownloadTaskResponse,
                           download::v1::CreateLinkDownloadTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::CreateLinkDownloadTaskRequest& request) {
             return download_service_stub_->PrepareAsyncCreateLinkDownloadTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;   
    case HandlerRequestId::kCreateTorrentDownloadTask:
      (new AsyncClientCall<download::v1::CreateTorrentDownloadTaskReply,
                           download::v1::CreateTorrentDownloadTaskRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::CreateTorrentDownloadTaskRequest& request) {
             return download_service_stub_->PrepareAsyncCreateTorrentDownloadTask(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kUploadTorrentFile:
      (new AsyncClientCall<download::v1::UploadTorrentFileReply,
                           download::v1::UploadTorrentFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::UploadTorrentFileRequest& request) {
             return download_service_stub_->PrepareAsyncUploadTorrentFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;      
    case HandlerRequestId::kPause:
      (new AsyncClientCall<download::v1::PauseReply,
                           download::v1::PauseRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::PauseRequest& request) {
             return download_service_stub_->PrepareAsyncPause(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kPauseAll:
      (new AsyncClientCall<download::v1::PauseAllReply,
                           download::v1::PauseAllRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::PauseAllRequest& request) {
             return download_service_stub_->PrepareAsyncPauseAll(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kResume:
      (new AsyncClientCall<download::v1::ResumeReply,
                           download::v1::ResumeRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::ResumeRequest& request) {
             return download_service_stub_->PrepareAsyncResume(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kResumeAll:
      (new AsyncClientCall<download::v1::ResumeAllReply,
                           download::v1::ResumeAllRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::ResumeAllRequest& request) {
             return download_service_stub_->PrepareAsyncResumeAll(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kDelete:
      (new AsyncClientCall<download::v1::DeleteReply,
                           download::v1::DeleteRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::DeleteRequest& request) {
             return download_service_stub_->PrepareAsyncDelete(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kDeleteCategory:
      (new AsyncClientCall<download::v1::DeleteCategoryReply,
                           download::v1::DeleteCategoryRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::DeleteCategoryRequest& request) {
             return download_service_stub_->PrepareAsyncDeleteCategory(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kRetry:
      (new AsyncClientCall<download::v1::RetryReply,
                           download::v1::RetryRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::RetryRequest& request) {
             return download_service_stub_->PrepareAsyncRetry(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kRetryAll:
      (new AsyncClientCall<download::v1::RetryAllReply,
                           download::v1::RetryAllRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::RetryAllRequest& request) {
             return download_service_stub_->PrepareAsyncRetryAll(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kClean:
      (new AsyncClientCall<download::v1::CleanReply,
                           download::v1::CleanRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::CleanRequest& request) {
             return download_service_stub_->PrepareAsyncClean(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kCleanCategory:
      (new AsyncClientCall<download::v1::CleanCategoryReply,
                           download::v1::CleanCategoryRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::CleanCategoryRequest& request) {
             return download_service_stub_->PrepareAsyncCleanCategory(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kRestore:
      (new AsyncClientCall<download::v1::RestoreReply,
                           download::v1::RestoreRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::RestoreRequest& request) {
             return download_service_stub_->PrepareAsyncRestore(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kRestoreAll:
      (new AsyncClientCall<download::v1::RestoreAllReply,
                           download::v1::RestoreAllRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::RestoreAllRequest& request) {
             return download_service_stub_->PrepareAsyncRestoreAll(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetTaskInfo:
      (new AsyncClientCall<download::v1::GetReply,
                           download::v1::GetRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::GetRequest& request) {
             return download_service_stub_->PrepareAsyncGet(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetLinkInfo:
      (new AsyncClientCall<download::v1::ParseLinkReply,
                           download::v1::ParseLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::ParseLinkRequest& request) {
             return download_service_stub_->PrepareAsyncParseLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetTorrentInfo:
      (new AsyncClientCall<download::v1::ParseTorrentReply,
                           download::v1::ParseTorrentRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::ParseTorrentRequest& request) {
             return download_service_stub_->PrepareAsyncParseTorrent(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetTorrentUploadPath:
      (new AsyncClientCall<download::v1::GetTorrentUploadPathReply,
                           download::v1::GetTorrentUploadPathRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const download::v1::GetTorrentUploadPathRequest& request) {
             return download_service_stub_->PrepareAsyncGetTorrentUploadPath(
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
bool DownloadServerStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  
  AsyncInitStub(base::BindOnce(&DownloadServerStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void DownloadServerStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](DownloadServerStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->download_service_stub_ =
                download::v1::DownloadService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}

}  // namespace nas