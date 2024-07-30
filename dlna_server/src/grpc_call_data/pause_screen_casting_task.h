// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:42
#ifndef NAS_DLNA_SERVER_SRC_GRPC_CALL_DATA_PAUSE_SCREEN_CASTING_TASK_H_
#define NAS_DLNA_SERVER_SRC_GRPC_CALL_DATA_PAUSE_SCREEN_CASTING_TASK_H_

#include "nas/common/call_result.h"
#include "nas/common/context_field.h"
#include "nas/common/grpc_server_call_data.h"

#include "base/logging.h"
#include "nas/dlna_server/src/dlna_server_async_impl.h"

namespace nas {

class PauseScreenCastingTaskCallData
    : public nas::GrpcServerCallDataBase<DlnaServiceAsyncImpl,
                                         PauseScreenCastingTaskCallData> {
 public:
  PauseScreenCastingTaskCallData(DlnaServiceAsyncImpl* service,
                                 grpc::ServerCompletionQueue* cq,
                                 nas::NasThread* cq_dispatch,
                                 nas::NasThread* task_main)
      : nas::GrpcServerCallDataBase<DlnaServiceAsyncImpl,
                                    PauseScreenCastingTaskCallData>(service,
                                                                    cq,
                                                                    cq_dispatch,
                                                                    task_main),
        responder_(&ctx_),
        async_service_(service) {
    screen_casting_mgr_ = async_service_->GetScreenCastingMgrPtr();
  }

 protected:
  void Request(DlnaServiceAsyncImpl* service,
               grpc::ServerCompletionQueue* cq) override {
    if (service && cq) {
      service->RequestPauseScreenCastingTask(&ctx_, &request_, &responder_, cq,
                                             cq, this);
    }
  }

  void Process() override;
  void ProcessComplete();
  void Finish() override {
    call_result_.msg =
        nas::GetErrorDesc((DlnaServiceErrorNo)call_result_.errcode);
    LOG(WARNING) << "on PauseScreenCastingTaskCallData, errcode: "
                 << call_result_.errcode;
    nas::WrapperSetResult(&ctx_).SetResult(call_result_);
    responder_.Finish(response_, grpc::Status::OK, this);
  }

 private:
  DlnaServiceAsyncImpl* async_service_ = nullptr;
  grpc::ServerContext ctx_;
  dlna::v1::PauseScreenCastingTaskRequest request_;
  dlna::v1::PauseScreenCastingTaskResponse response_;
  grpc::ServerAsyncResponseWriter<dlna::v1::PauseScreenCastingTaskResponse>
      responder_;
  nas::CallResult call_result_;

  std::shared_ptr<ScreenCastingMgr> screen_casting_mgr_;
};

}  // namespace nas

#endif  // NAS_DLNA_SERVER_SRC_GRPC_CALL_DATA_PAUSE_SCREEN_CASTING_TASK_H_
