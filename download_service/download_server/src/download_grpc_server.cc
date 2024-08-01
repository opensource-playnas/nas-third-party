/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:51:59
 */
#include "download_grpc_server.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"

#include "nas/common/call_result.h"
#include "nas/common/context_field.h"
#include "nas/common/grpc_health_check.hpp"
#include "nas/common/micro_service_info.hpp"
#include "nas/common/path/path_protected.h"

#include "grpc_call_data/create_link_call_data.h"
#include "grpc_call_data/parse_link_call_data.h"
#include "grpc_call_data/upload_torrent_call_data.h"
#include "grpc_call_data/create_torrent_call_data.h"
#include "grpc_call_data/parse_torrent_call_data.h"
#include "grpc_call_data/pause_call_data.h"
#include "grpc_call_data/pause_all_call_data.h"
#include "grpc_call_data/resume_call_data.h"
#include "grpc_call_data/resume_all_call_data.h"
#include "grpc_call_data/delete_call_data.h"
#include "grpc_call_data/delete_category_call_data.h"
#include "grpc_call_data/get_call_data.h"
#include "grpc_call_data/retry_call_data.h"
#include "grpc_call_data/retry_all_call_data.h"
#include "grpc_call_data/restore_call_data.h"
#include "grpc_call_data/restore_all_call_data.h"
#include "grpc_call_data/clean_call_data.h"
#include "grpc_call_data/clean_category_call_data.h"
#include "grpc_call_data/get_torrent_upload_path_call_data.h"

#include "utils/utils.h"

namespace nas {

DownloadServiceImpl::DownloadServiceImpl() {

  cq_dispatch_thread_ = std::make_shared<NasThread>(kGrpcCQDispatchThreadName);
  task_main_thread_ = std::make_shared<NasThread>(kGrpcTaskMainThreadName);

  nas::path_unit::ProtectedPath::GetInstance()->Init();
  download_mgr_ = std::make_shared<DownloadManager>();
  if (download_mgr_) {
    download_mgr_->Init();
  }
  parse_mgr_ = std::make_shared<ParseManager>();
}

DownloadServiceImpl::~DownloadServiceImpl() {}

bool DownloadServiceImpl::RunServer() {
  std::string download_server_address("127.0.0.1:0");

  grpc::EnableDefaultHealthCheckService(true);
  // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  int listen_port = 0;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(download_server_address,
                           grpc::InsecureServerCredentials(), &listen_port);
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(this);
  // register grpc health check
  nas::GrpcHealthRegister(kDownloadServiceName, builder);


  // Get hold of the completion queue used for the asynchronous communication
  // with the gRPC runtime.
  cq_ = builder.AddCompletionQueue();

  server_ = builder.BuildAndStart();

   std::string port = std::to_string(listen_port);
   download_server_address.replace(download_server_address.end() - 1,
                                   download_server_address.end(), port);

  LOG(INFO) << "download server listening on " << download_server_address;


  CreateCallDatas();
  nas::HandleRpcRequests(cq_.get());
  LOG(INFO) << "service END";
  return true;
}


void DownloadServiceImpl::CreateCallDatas() {
  NewCallData<DownloadServiceImpl,
              nas::CreateLinkDownloadTaskCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::ParseLinkCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::UploadTorrentFileCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::CreateTorrentDownloadTaskCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::ParseTorrentCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::PauseCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::PauseAllCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::ResumeCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::ResumeAllCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::DeleteCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::DeleteCategoryCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::GetCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::RetryCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::RetryAllCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::RestoreCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::RestoreAllCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::CleanCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::CleanCategoryCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
  NewCallData<DownloadServiceImpl,
              nas::GetTorrentUploadPathCallData>(
      this, cq_.get(), cq_dispatch_thread_.get(), task_main_thread_.get());
}

std::shared_ptr<DownloadManager> DownloadServiceImpl::GetDownloadMgr() {
  return download_mgr_;
}

std::shared_ptr<ParseManager> DownloadServiceImpl::GetParseMgr() {
  return parse_mgr_;
}

void DownloadServiceImpl::Stop() {
  if (cq_) {
    cq_->Shutdown();
  }

  task_main_thread_->Stop();
  cq_dispatch_thread_->Stop();

  if (download_mgr_) {
    download_mgr_->UnInit();
  }

  nas::path_unit::ProtectedPath::GetInstance()->Uninit();
  if (server_) {
    server_->Shutdown();
  }
}

void DownloadServiceImpl::Reset(int level) {
  LOG(WARNING) << "reset start!";
  base::WaitableEvent wait_event;
  if (download_mgr_) {
    download_mgr_->Reset(wait_event);
  }
  wait_event.Wait();
  LOG(WARNING) << "reset finished!";
}

}  // namespace nas
