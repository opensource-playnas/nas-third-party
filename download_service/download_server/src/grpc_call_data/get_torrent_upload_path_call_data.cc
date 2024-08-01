#include "get_torrent_upload_path_call_data.h"
#include "nas/download_service/download_server/src/download_mgr.h"


namespace nas {

GetTorrentUploadPathCallData::GetTorrentUploadPathCallData(
    nas::DownloadServiceImpl* service,
    ServerCompletionQueue* cq,
    NasThread* cq_dispatch,
    NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                              GetTorrentUploadPathCallData>(service,
                                                              cq,
                                                              cq_dispatch,
                                                              task_main),
      responder_(&ctx_) {}

void GetTorrentUploadPathCallData::Request(
    DownloadServiceImpl* service,
    ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestGetTorrentUploadPath(&ctx_, &request_, &responder_, cq,
                                           cq, tag_);
  }
}

void GetTorrentUploadPathCallData::Process() {
  nas::ContextField context_field(&ctx_);
  std::string torrent_path;
  if (service_) {
    std::shared_ptr<nas::DownloadManager> download_mgr =
        service_->GetDownloadMgr();
    if (download_mgr) {
      torrent_path = download_mgr->GetTorrentUploadPath(context_field);
    }

  }

  response_.set_torrent_path(torrent_path);
  nas::WrapperSetResult wr(&ctx_);
  wr.SetResult(call_request_);
  OnProcessed();
}

void GetTorrentUploadPathCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas