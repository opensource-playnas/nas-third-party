/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-07 17:08:21
 */

#include "file_service_stub.h"

#include "google/protobuf/util/json_util.h"
#include "nas/common/micro_service_info.hpp"

#include "grpc_client_context.hpp"

namespace nas {
FileServerStub::FileServerStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kFileServiceName;
}

void FileServerStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;
  bool processed = true;
  switch (handler_id_) {    
    case HandlerRequestId::kDiskSize:
      (new AsyncClientCall<file::v1::GetDiskSizeResponse,
                           file::v1::GetDiskSizeRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::GetDiskSizeRequest& request) {
             return file_service_stub_->PrepareAsyncGetDiskSize(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileCreate:
      (new AsyncClientCall<file::v1::CreateResponse,
                           file::v1::CreateRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::CreateRequest& request) {
             return file_service_stub_->PrepareAsyncCreate(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileList:
      (new AsyncClientCall<file::v1::ListFilesAndDirectoriesResponse,
                           file::v1::ListFilesAndDirectoriesRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::ListFilesAndDirectoriesRequest& request) {
             return file_service_stub_->PrepareAsyncListFilesAndDirectories(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFolderDetails:
      (new AsyncClientCall<file::v1::GetPathAttributeResponse,
                           file::v1::GetPathAttributeRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::GetPathAttributeRequest& request) {
             return file_service_stub_->PrepareAsyncGetPathAttribute(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileDelete:
      (new AsyncClientCall<file::v1::DeleteResponse,
                           file::v1::DeleteRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::DeleteRequest& request) {
             return file_service_stub_->PrepareAsyncDelete(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileRename:
      (new AsyncClientCall<file::v1::RenameResponse,
                           file::v1::RenameRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::RenameRequest& request) {
             return file_service_stub_->PrepareAsyncRename(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileMove:
      (new AsyncClientCall<file::v1::MoveResponse,
                           file::v1::MoveRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::MoveRequest& request) {
             return file_service_stub_->PrepareAsyncMove(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileCopy:
      (new AsyncClientCall<file::v1::CopyResponse,
                           file::v1::CopyRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::CopyRequest& request) {
             return file_service_stub_->PrepareAsyncCopy(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileSearch:
      (new AsyncClientCall<file::v1::SearchFileResponse,
                           file::v1::SearchFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::SearchFileRequest& request) {
             return file_service_stub_->PrepareAsyncSearchFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFileOperatorCacel:
      (new AsyncClientCall<file::v1::CancelFileOperateResponse,
                           file::v1::CancelFileOperateRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::CancelFileOperateRequest& request) {
             return file_service_stub_->PrepareAsyncCancelFileOperate(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFavoriteAdd:
      (new AsyncClientCall<file::v1::AddMyCollectionResponse,
                           file::v1::AddMyCollectionRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::AddMyCollectionRequest& request) {
             return file_service_stub_->PrepareAsyncAddMyCollection(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFavoriteList:
      (new AsyncClientCall<file::v1::GetMyCollectionResponse,
                           file::v1::GetMyCollectionRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::GetMyCollectionRequest& request) {
             return file_service_stub_->PrepareAsyncGetMyCollection(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFavoriteMove:
      (new AsyncClientCall<file::v1::MoveMyCollectionResponse,
                           file::v1::MoveMyCollectionRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::MoveMyCollectionRequest& request) {
             return file_service_stub_->PrepareAsyncMoveMyCollection(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kFavoriteCancel:
      (new AsyncClientCall<file::v1::CancelMyCollectionResponse,
                           file::v1::CancelMyCollectionRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::CancelMyCollectionRequest& request) {
             return file_service_stub_->PrepareAsyncCancelMyCollection(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kCreateShareLink:
      (new AsyncClientCall<file::v1::CreateShareLinkResponse,
                           file::v1::CreateShareLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::CreateShareLinkRequest& request) {
             return file_service_stub_->PrepareAsyncCreateShareLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kDeleteShareLink:
      (new AsyncClientCall<file::v1::DeleteShareLinkResponse,
                           file::v1::DeleteShareLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::DeleteShareLinkRequest& request) {
             return file_service_stub_->PrepareAsyncDeleteShareLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kDeleteInvalidShareLink:
      (new AsyncClientCall<file::v1::DeleteInvalidShareLinkResponse,
                           file::v1::DeleteInvalidShareLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::DeleteInvalidShareLinkRequest& request) {
             return file_service_stub_->PrepareAsyncDeleteInvalidShareLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetShareLink:
      (new AsyncClientCall<file::v1::GetShareLinkResponse,
                           file::v1::GetShareLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::GetShareLinkRequest& request) {
             return file_service_stub_->PrepareAsyncGetShareLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kListShareLink:
      (new AsyncClientCall<file::v1::ListShareLinkResponse,
                           file::v1::ListShareLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::ListShareLinkRequest& request) {
             return file_service_stub_->PrepareAsyncListShareLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kResetShareLink:
      (new AsyncClientCall<file::v1::ResetShareLinkResponse,
                           file::v1::ResetShareLinkRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::ResetShareLinkRequest& request) {
             return file_service_stub_->PrepareAsyncResetShareLink(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
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
    case HandlerRequestId::kPathListExists:
      (new AsyncClientCall<file::v1::PathListExistsResponse,
                           file::v1::PathListExistsRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::PathListExistsRequest& request) {
             return file_service_stub_->PrepareAsyncPathListExists(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetIDByPath:
      (new AsyncClientCall<file::v1::GetIDByPathResponse,
                           file::v1::GetIDByPathRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::GetIDByPathRequest& request) {
             return file_service_stub_->PrepareAsyncGetIDByPath(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetPathByID:
      (new AsyncClientCall<file::v1::GetPathByIDResponse,
                           file::v1::GetPathByIDRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::GetPathByIDRequest& request) {
             return file_service_stub_->PrepareAsyncGetPathByID(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;      
    case HandlerRequestId::kFilePack:
      (new AsyncClientCall<file::v1::PackFileResponse,
                           file::v1::PackFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::PackFileRequest& request) {
             return file_service_stub_->PrepareAsyncPackFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;    
    case HandlerRequestId::kFileUnPack:
      (new AsyncClientCall<file::v1::UnpackFileResponse,
                           file::v1::UnpackFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::UnpackFileRequest& request) {
             return file_service_stub_->PrepareAsyncUnpackFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kArciveInfo:
      (new AsyncClientCall<file::v1::ReadArchiveInfoResponse,
                           file::v1::ReadArchiveInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::ReadArchiveInfoRequest& request) {
             return file_service_stub_->PrepareAsyncReadArchiveInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
     case HandlerRequestId::kGetSpecifiedTypeFiles:
      (new AsyncClientCall<file::v1::ListSpecifiedTpyeFilesResponse,
                           file::v1::ListSpecifiedTpyeFilesRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const file::v1::ListSpecifiedTpyeFilesRequest& request) {
             return file_service_stub_->PrepareAsyncListSpecifiedTpyeFiles(
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
bool FileServerStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);

  AsyncInitStub(base::BindOnce(&FileServerStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void FileServerStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](FileServerStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->file_service_stub_ =
                file::v1::FileStationService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas
