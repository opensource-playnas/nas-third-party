#include "clean_category_call_data.h"
#include "nas/download_service/download_server/src/download_mgr.h"


namespace nas {

CleanCategoryCallData::CleanCategoryCallData(
    nas::DownloadServiceImpl* service,
    ServerCompletionQueue* cq,
    NasThread* cq_dispatch,
    NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                              CleanCategoryCallData>(service,
                                                              cq,
                                                              cq_dispatch,
                                                              task_main),
      responder_(&ctx_) {}

void CleanCategoryCallData::Request(
    DownloadServiceImpl* service,
    ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestCleanCategory(&ctx_, &request_, &responder_, cq,
                                           cq, tag_);
  }
}

void CleanCategoryCallData::Process() {
  nas::ContextField context_field(&ctx_);

  if (service_) {
    std::shared_ptr<nas::DownloadManager> download_mgr =
        service_->GetDownloadMgr();
    if (download_mgr) {
      download_mgr->CleanCategory(&request_, context_field);
    }
  }

  nas::WrapperSetResult wr(&ctx_);
  wr.SetResult(call_request_);
  OnProcessed();
}

void CleanCategoryCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas