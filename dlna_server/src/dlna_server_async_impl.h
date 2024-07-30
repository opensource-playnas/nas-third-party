// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/08/21 15:01
#ifndef NAS_DLNA_SERVER_SRC_MEDIA_SERVER_ASYNC_IMPL_H_
#define NAS_DLNA_SERVER_SRC_MEDIA_SERVER_ASYNC_IMPL_H_

#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "nas/common/grpc_base_service/grpc_base_interface.h"
#include "nas/common/nas_thread.h"

#include "nas/dlna_server/protos/dlna_server.grpc.pb.h"

#include "nas/common/grpc_base_service/grpc_base_interface.h"

#include "Platinum.h"
#include "PltVersion.h"

#include "screen_casting/screen_casting_mgr.h"
#include "error_desc.h"

namespace nas {

class DlnaServiceAsyncImpl final
    : public dlna::v1::DlnaService::AsyncService,
      public nas::GrpcBaseServiceInterface {
 public:
  DlnaServiceAsyncImpl();
  ~DlnaServiceAsyncImpl();

  bool RunServer(std::string dlna_server_address);

 public:
  void Stop() override;
  void Reset(int level) override;
  std::shared_ptr<ScreenCastingMgr> GetScreenCastingMgrPtr() {
    return screen_casting_mgr_;
  }

 private:
  void CreateCallDatas();

 private:
  std::shared_ptr<ScreenCastingMgr> screen_casting_mgr_;

  std::unique_ptr<grpc::Server> server_ = nullptr;
  std::unique_ptr<grpc::ServerCompletionQueue> cq_;
  nas::NasThread grpc_request_dispatch_;
  nas::NasThread grpc_task_main_;

  PLT_UPnP upnp_;
  PLT_CtrlPointReference ctrl_point_;

  NasThread noraml_thread_;
  NasThread screen_casting_mgr_thread_;
};

}  // namespace nas

#endif  // NAS_DLNA_SERVER_SRC_MEDIA_SERVER_ASYNC_IMPL_H_
