// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/08/21 15:01

#include "dlna_server_async_impl.h"

#include "base/logging.h"

#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/escape.h"
#include "base/strings/string_split.h"

#include "nas/common/grpc_base_service/grpc_base_service_impl.h"
#include "nas/common/grpc_health_check.hpp"
#include "nas/common/grpc_server_call_data.h"
#include "nas/common/micro_service_info.hpp"

#include "nas/common/nas_thread.h"
#include "public_define.h"

#include "grpc_call_data/cancel_screen_casting_task.h"
#include "grpc_call_data/change_screen_casting_task_play_url.h"
#include "grpc_call_data/create_screen_casting_task.h"
#include "grpc_call_data/list_screen_casting_devices.h"
#include "grpc_call_data/pause_screen_casting_task.h"
#include "grpc_call_data/play_screen_casting_task.h"
#include "grpc_call_data/seek_screen_casting_task.h"
#include "grpc_call_data/set_screen_casting_task_volume.h"

namespace nas {

DlnaServiceAsyncImpl::DlnaServiceAsyncImpl()
    : grpc_request_dispatch_(kGrpcCQDispatchThreadName),
      grpc_task_main_(kGrpcTaskMainThreadName),
      noraml_thread_(kNormalThreadName),
      screen_casting_mgr_thread_(kScreenCastingMgrThreadName),
      ctrl_point_(new PLT_CtrlPoint()) {

  screen_casting_mgr_ = std::make_shared<ScreenCastingMgr>(
      &screen_casting_mgr_thread_, ctrl_point_);
  // add control point to upnp engine and start it

  upnp_.AddCtrlPoint(ctrl_point_);
  upnp_.Start();
  // tell control point to perform extra broadcast discover every 6 secs
  // in case our device doesn't support multicast
  ctrl_point_->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"),
                        "upnp:rootdevice", 1, NPT_TimeStamp(6.));
}

DlnaServiceAsyncImpl::~DlnaServiceAsyncImpl() {
}

bool DlnaServiceAsyncImpl::RunServer(std::string dlna_server_address) {
  grpc::EnableDefaultHealthCheckService(true);
  // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  int listen_port = 0;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(dlna_server_address,
                           grpc::InsecureServerCredentials(), &listen_port);
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(this);
  // register grpc health check
  nas::GrpcHealthRegister(kDlnaService, builder);

  nas::GrpcBaseServiceImpl grpc_base_service_impl(this);
  builder.RegisterService(&grpc_base_service_impl);
  cq_ = builder.AddCompletionQueue();
  // Finally assemble the server.
  server_ = builder.BuildAndStart();

  auto tmp_str =
      base::SplitString(dlna_server_address, ":", base::TRIM_WHITESPACE,
                        base::SPLIT_WANT_NONEMPTY);
  if (tmp_str.size() > 1) {
    std::string port = std::to_string(listen_port);
    dlna_server_address = tmp_str.at(0) + ":" + port;
  }

  LOG(INFO) << "dlna server listening on " << dlna_server_address;

  CreateCallDatas();
  nas::HandleRpcs(cq_.get());
  LOG(INFO) << "service END";
  return true;
}

void DlnaServiceAsyncImpl::Stop() {
  if (cq_) {
    cq_->Shutdown();
  }
  // 等待所有线程池任务完成
  grpc_task_main_.Stop();
  // 等待所有request完成
  // 退出request调度线程
  grpc_request_dispatch_.Stop();

  if (server_) {
    server_->Shutdown();
  }
  LOG(INFO) << "stoped";
}

void DlnaServiceAsyncImpl::Reset(int level) {
  LOG(INFO) << "Reset: " << level;
  // 手动取消所有的投屏任务
  screen_casting_mgr_->CancelAllTask();
}

void DlnaServiceAsyncImpl::CreateCallDatas() {
  nas::NewCallData<DlnaServiceAsyncImpl, ListScreenCastingDevicesCallDate>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl, CreateScreenCastingTaskCallDate>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl, CancelScreenCastingTaskCallDate>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl,
                   ChangeScreenCastingTaskPlayUrlCallDate>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl, PlayScreenCastingTaskCallDate>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl, PauseScreenCastingTaskCallData>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl, SeekScreenCastingTaskCallDate>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
  nas::NewCallData<DlnaServiceAsyncImpl, SetScreenCastingTaskVolumeCallData>(
      this, cq_.get(), &grpc_request_dispatch_, &grpc_task_main_);
}

}  // namespace nas
