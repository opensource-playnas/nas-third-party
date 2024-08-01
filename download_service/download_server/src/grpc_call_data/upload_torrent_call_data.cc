#include "upload_torrent_call_data.h"
#include "nas/download_service/download_server/src/download_mgr.h"


namespace nas {

UploadTorrentFileCallData::UploadTorrentFileCallData(
    nas::DownloadServiceImpl* service,
    ServerCompletionQueue* cq,
    NasThread* cq_dispatch,
    NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                              UploadTorrentFileCallData>(service,
                                                              cq,
                                                              cq_dispatch,
                                                              task_main),
      responder_(&ctx_) {}

void UploadTorrentFileCallData::Request(
    DownloadServiceImpl* service,
    ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestUploadTorrentFile(&ctx_, &request_, &responder_, cq,
                                           cq, tag_);
  }
}

void UploadTorrentFileCallData::AsynUpload() {
  nas::ContextField context_field(&ctx_);
  std::string torrent_path;
  if (service_) {
    std::shared_ptr<nas::DownloadManager> download_mgr =
        service_->GetDownloadMgr();
    if (download_mgr) {
      torrent_path = download_mgr->UploadTorrentFile(&request_);
    }

  }
  if (torrent_path.empty()) {
    DownloadErrorCode err = DownloadErrorCode::kUploadFileFailed;
    call_request_.errcode = err;
    call_request_.msg = nas::GetDownloadErrorDesc(err);
  }

  response_.set_torrent_path(torrent_path);
  nas::WrapperSetResult wr(&ctx_);
  wr.SetResult(call_request_);
  OnProcessed();
}


void UploadTorrentFileCallData::Process() {
  base::ThreadPool::PostTask(
      FROM_HERE, base::MayBlock(),
      base::BindOnce(&UploadTorrentFileCallData::AsynUpload,
                     base::Unretained(this)));
}

void UploadTorrentFileCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas