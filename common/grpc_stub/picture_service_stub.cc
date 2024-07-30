// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-07-17 19:15:12

#include "picture_service_stub.h"

#include <functional>

#include "nas/common/micro_service_info.hpp"
#include "grpc_client_context.hpp"

namespace nas {

PictureServiceStub::PictureServiceStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kPictureServiceName;
}


void PictureServiceStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){
  grpc::CompletionQueue* grpc_completion_queue =
      grpc_completion_queue_;
  bool processed = true;
  switch (handler_id_) {
    case HandlerRequestId::kPictureThumbnailPath:
      (new AsyncClientCall<picture::v1::ThumbFilePathResponse,
                            picture::v1::ThumbFilePathRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::ThumbFilePathRequest& request) {
             return picture_service_stub_->PrepareAsyncThumbFilePath(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperCollectionList:
      (new AsyncClientCall<picture::v1::WallpaperCollectionListResponse,
                            picture::v1::WallpaperCollectionListRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperCollectionListRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperCollectionList(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperAddCollection:
      (new AsyncClientCall<picture::v1::WallpaperAddCollectionResponse,
                            picture::v1::WallpaperAddCollectionRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperAddCollectionRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperAddCollection(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperDelCollection:
      (new AsyncClientCall<picture::v1::WallpaperDeleteCollectionResponse,
                            picture::v1::WallpaperDeleteCollectionRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperDeleteCollectionRequest& request) {
             return picture_service_stub_
                 ->PrepareAsyncWallpaperDeleteCollection(
                     &client_context_->ClientContext(), request,
                     grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperFileList:
      (new AsyncClientCall<picture::v1::WallpaperFileListResponse,
                            picture::v1::WallpaperFileListRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperFileListRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperFileList(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperAddFile:
      (new AsyncClientCall<picture::v1::WallpaperAddFileResponse,
                            picture::v1::WallpaperAddFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperAddFileRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperAddFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperDelFile:
      (new AsyncClientCall<picture::v1::WallpaperDeleteFileResponse,
                            picture::v1::WallpaperDeleteFileRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperDeleteFileRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperDeleteFile(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperSetSetting:
      (new AsyncClientCall<picture::v1::WallpaperSetSettingResponse,
                            picture::v1::WallpaperSetSettingRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperSetSettingRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperSetSetting(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureWallpaperGetSetting:
      (new AsyncClientCall<picture::v1::WallpaperGetSettingResponse,
                            picture::v1::WallpaperGetSettingRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::WallpaperGetSettingRequest& request) {
             return picture_service_stub_->PrepareAsyncWallpaperGetSetting(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureFileTranscode:
      (new AsyncClientCall<picture::v1::TranscodeImageResponse,
                            picture::v1::TranscodeImageRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::TranscodeImageRequest& request) {
             return picture_service_stub_->PrepareAsyncTranscodeImage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kPictureGalleryGetMetaInfo:
      (new AsyncClientCall<picture::v1::QueryPictureFileInfoResponse,
                            picture::v1::QueryPictureFileInfoRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this, grpc_completion_queue](
               const picture::v1::QueryPictureFileInfoRequest& request) {
             return picture_service_stub_->PrepareAsyncQueryPictureFileInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue);
           }))
          ->Call(param);
      break;
    default: {
      processed = false;
      break;
    }
  }
  DCHECK(processed);
}
bool PictureServiceStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  
  AsyncInitStub(base::BindOnce(&PictureServiceStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void PictureServiceStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](PictureServiceStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->picture_service_stub_ = picture::v1::PictureService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas