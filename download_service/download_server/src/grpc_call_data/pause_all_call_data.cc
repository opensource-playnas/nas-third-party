#include "pause_all_call_data.h"
#include "nas/download_service/download_server/src/download_mgr.h"


namespace nas {

PauseAllCallData::PauseAllCallData(
    nas::DownloadServiceImpl* service,
    ServerCompletionQueue* cq,
    NasThread* cq_dispatch,
    NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                              PauseAllCallData>(service,
                                                              cq,
                                                              cq_dispatch,
                                                              task_main),
      responder_(&ctx_) {}

void PauseAllCallData::Request(
    DownloadServiceImpl* service,
    ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestPauseAll(&ctx_, &request_, &responder_, cq,
                                           cq, tag_);
  }
}

void PauseAllCallData::Process() {
  nas::ContextField context_field(&ctx_);

  if (service_) {
    std::shared_ptr<nas::DownloadManager> download_mgr =
        service_->GetDownloadMgr();
    if (download_mgr) {
      download_mgr->PauseAll(context_field);
    }
  }

  nas::WrapperSetResult wr(&ctx_);
  wr.SetResult(call_request_);
  OnProcessed();
}

void PauseAllCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas