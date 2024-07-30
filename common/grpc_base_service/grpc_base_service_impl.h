/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-22 15:01:11
 */
#ifndef NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_SERVICE_IMPL_H_
#define NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_SERVICE_IMPL_H_

#include "base/threading/thread.h"
#include "grpcpp/grpcpp.h"
#include "nas/common/grpc_base_service/protos/grpc_base_service.grpc.pb.h"
#include "grpc_base_interface.h"

using grpc::ServerContext;
using grpc::Status;
using nas::GrpcBaseService;
using nas::StopRequest;
using nas::StopResponse;
using nas::ResetRequest;
using nas::ResetResponse;

namespace nas {
class GrpcBaseServiceImpl final : public nas::GrpcBaseService::Service {
 public:
  explicit GrpcBaseServiceImpl(GrpcBaseServiceInterface* server_impl);
  ~GrpcBaseServiceImpl();

 private:
  Status Stop(ServerContext* context,
              const StopRequest* request,
              StopResponse* response) override;

  Status Reset(ServerContext* context,
               const ResetRequest* request,
               ResetResponse* response) override;
  GrpcBaseServiceInterface* server_impl_ = nullptr;
  base::Thread work_thread_;
};  //
}  // namespace nas
#endif  // NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_SERVICE_IMPL_H_