/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-24 15:40:23
 */
#ifndef NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_SERVICE_STUB_H_
#define NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_SERVICE_STUB_H_

#include "base/logging.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "nas/common/grpc_base_service/protos/grpc_base_service.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace nas {

class GrpcBaseServiceStub {
 public:
  GrpcBaseServiceStub() {}
  void Stop(const std::string& service_name, const std::string& ip_port) {
    std::shared_ptr<Channel> channel =
        grpc::CreateChannel(ip_port, grpc::InsecureChannelCredentials());
    std::unique_ptr<GrpcBaseService::Stub> stub =
        nas::GrpcBaseService::NewStub(channel);
    nas::StopRequest request;
    nas::StopResponse response;
    grpc::ClientContext context;
    grpc::Status s = stub->Stop(&context, request, &response);
    if (!s.ok()) {
      LOG(ERROR) << "stop error:" << service_name;
    }
    return;
  }

  void Reset(const std::string& service_name, const std::string& ip_port) {
    std::shared_ptr<Channel> channel =
        grpc::CreateChannel(ip_port, grpc::InsecureChannelCredentials());
    std::unique_ptr<GrpcBaseService::Stub> stub =
        nas::GrpcBaseService::NewStub(channel);
    nas::ResetRequest request;
    nas::ResetResponse response;
    grpc::ClientContext context;
    grpc::Status s = stub->Reset(&context, request, &response);
    if (!s.ok()) {
      LOG(ERROR) << "reset error:" << service_name;
    }
    return;
  }
};  // GrpcBaseServiceStub
}  // namespace nas

#endif  // NAS_COMMON_GRPC_BASE_SERVICE_GRPC_BASE_SERVICE_STUB_H_