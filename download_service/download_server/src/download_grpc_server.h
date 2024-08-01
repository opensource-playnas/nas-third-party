/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:51:28
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_GRPC_SERVER_H_
#define NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_GRPC_SERVER_H_

#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/security/credentials.h>

#include "nas/common/grpc_base_service/grpc_base_interface.h"
#include "nas/common/grpc_server_call_data.h"
#include "nas/common/nas_thread.h"
#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"


#include "download_mgr.h"
#include "parse_mgr.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace nas {

class DownloadServiceImpl final : public download::v1::DownloadService::AsyncService,
                                  public nas::GrpcBaseServiceInterface {
 public:
  DownloadServiceImpl();
  virtual ~DownloadServiceImpl();

  bool RunServer();

 public:
  void Stop() override;
  void Reset(int level) override;

 public:
  std::shared_ptr<DownloadManager> GetDownloadMgr();
  std::shared_ptr<ParseManager> GetParseMgr();

 private:
  void CreateCallDatas();

 private:
  std::shared_ptr<DownloadManager> download_mgr_ = nullptr;
  std::shared_ptr<ParseManager> parse_mgr_ = nullptr;
  std::unique_ptr<Server> server_ = nullptr;
  std::shared_ptr<DownloadServiceImpl> download_service_impl_ = nullptr;
  std::unique_ptr<grpc::ServerCompletionQueue> cq_;

  std::shared_ptr<NasThread> cq_dispatch_thread_ = nullptr;
  std::shared_ptr<NasThread> task_main_thread_ = nullptr;
};

}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_GRPC_SERVER_H_