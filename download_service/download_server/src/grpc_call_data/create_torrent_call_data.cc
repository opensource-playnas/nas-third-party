#include "create_torrent_call_data.h"
#include "nas/download_service/download_server/src/download_mgr.h"


namespace nas {

CreateTorrentDownloadTaskCallData::CreateTorrentDownloadTaskCallData(
    nas::DownloadServiceImpl* service,
    ServerCompletionQueue* cq,
    NasThread* cq_dispatch,
    NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                              CreateTorrentDownloadTaskCallData>(service,
                                                              cq,
                                                              cq_dispatch,
                                                              task_main),
      responder_(&ctx_) {}

void CreateTorrentDownloadTaskCallData::Request(
    DownloadServiceImpl* service,
    ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestCreateTorrentDownloadTask(&ctx_, &request_, &responder_, cq,
                                           cq, tag_);
  }
}

void CreateTorrentDownloadTaskCallData::Process() {
  nas::ContextField context_field(&ctx_);

  if (service_) {
    std::shared_ptr<nas::DownloadManager> download_mgr =
        service_->GetDownloadMgr();
    if (download_mgr) {
      download_mgr->CreateTorrentDownloadTask(&request_, context_field);
    }
  }

  nas::WrapperSetResult wr(&ctx_);
  wr.SetResult(call_request_);
  OnProcessed();
}

void CreateTorrentDownloadTaskCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas