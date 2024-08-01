#include "parse_link_call_data.h"
#include "nas/download_service/download_server/src/parse_mgr.h"

namespace nas {

ParseLinkCallData::ParseLinkCallData(nas::DownloadServiceImpl* service,
                                     ServerCompletionQueue* cq,
                                     NasThread* cq_dispatch,
                                     NasThread* task_main)
    : nas::GrpcServerCallBase<nas::DownloadServiceImpl, ParseLinkCallData>(
          service,
          cq,
          cq_dispatch,
          task_main),
      responder_(&ctx_) {}

void ParseLinkCallData::Request(DownloadServiceImpl* service,
                                ServerCompletionQueue* cq) {
  if (service && cq && tag_) {
    service->RequestParseLink(&ctx_, &request_, &responder_, cq, cq, tag_);
  }
}

void ParseLinkCallData::AsynParseLink() {
  nas::ContextField context_field(&ctx_);

  DownloadErrorCode err = DownloadErrorCode::kNormal;
  if (service_) {
    std::shared_ptr<nas::ParseManager> parse_mgr = service_->GetParseMgr();
    if (parse_mgr) {
      err = parse_mgr->ParseLink(&request_, &response_, context_field);
    }
  }

  nas::WrapperSetResult wr(&ctx_);
  nas::utils::SetRpcResult(err, &wr);
  OnProcessed();
}

void ParseLinkCallData::Process() {
  base::ThreadPool::PostTask(
      FROM_HERE, base::MayBlock(),
      base::BindOnce(&ParseLinkCallData::AsynParseLink,
                     base::Unretained(this)));
}

void ParseLinkCallData::Finish() {
  if (tag_) {
    responder_.Finish(response_, grpc::Status::OK, tag_);
  }
}

}  // namespace nas