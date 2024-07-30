/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-08-22 13:35:09
 */

#ifndef NAS_COMMON_GRPC_HEALTH_CHECK_H
#define NAS_COMMON_GRPC_HEALTH_CHECK_H
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "third_party/grpc/src/include/grpcpp/ext/health_check_service_server_builder_option.h"
#include "third_party/grpc/src/src/proto/grpc/health/v1/health.grpc.pb.h"
namespace nas {
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using grpc::health::v1::Health;
using grpc::health::v1::HealthCheckRequest;
using grpc::health::v1::HealthCheckResponse;

// A sample sync implementation of the health checking service. This does the
// same thing as the default one.
class HealthCheckServiceImpl : public Health::Service {
 public:
  Status Check(ServerContext* context,
               const HealthCheckRequest* request,
               HealthCheckResponse* response) override {
    std::lock_guard<std::mutex> lock(mu_);
    auto iter = status_map_.find(request->service());
    if (iter == status_map_.end()) {
      return Status(grpc::StatusCode::NOT_FOUND, "");
    }
    response->set_status(iter->second);
    return Status::OK;
  }
  Status Watch(ServerContext* context,
               const HealthCheckRequest* request,
               grpc::ServerWriter<HealthCheckResponse>* writer) override {
    auto last_state = HealthCheckResponse::UNKNOWN;
    while (!context->IsCancelled()) {
      {
        std::lock_guard<std::mutex> lock(mu_);
        HealthCheckResponse response;
        auto iter = status_map_.find(request->service());
        if (iter == status_map_.end()) {
          response.set_status(response.SERVICE_UNKNOWN);
        } else {
          response.set_status(iter->second);
        }
        if (response.status() != last_state) {
          writer->Write(response, grpc::WriteOptions());
          last_state = response.status();
        }
      }
      gpr_sleep_until(gpr_time_add(gpr_now(GPR_CLOCK_MONOTONIC),
                                   gpr_time_from_millis(1000, GPR_TIMESPAN)));
    }
    return Status::OK;
  }
  void SetStatus(const std::string& service_name,
                 HealthCheckResponse::ServingStatus status) {
    std::lock_guard<std::mutex> lock(mu_);
    if (shutdown_) {
      status = HealthCheckResponse::NOT_SERVING;
    }
    status_map_[service_name] = status;
  }
  void SetAll(HealthCheckResponse::ServingStatus status) {
    if (shutdown_) {
      return;
    }
    for (auto iter = status_map_.begin(); iter != status_map_.end(); ++iter) {
      iter->second = status;
    }
  }

  void Shutdown() {
    std::lock_guard<std::mutex> lock(mu_);
    if (shutdown_) {
      return;
    }
    shutdown_ = true;
    for (auto iter = status_map_.begin(); iter != status_map_.end(); ++iter) {
      iter->second = HealthCheckResponse::NOT_SERVING;
    }
  }

 private:
  std::mutex mu_;
  bool shutdown_ = false;
  std::map<const std::string, HealthCheckResponse::ServingStatus> status_map_;
};

// A custom implementation of the health checking service interface. This is
// used to test that it prevents the server from creating a default service and
// also serves as an example of how to override the default service.
class CustomHealthCheckService : public grpc::HealthCheckServiceInterface {
 public:
  explicit CustomHealthCheckService(HealthCheckServiceImpl* impl)
      : impl_(impl) {
    impl_->SetStatus("", HealthCheckResponse::SERVING);
  }
  void SetServingStatus(const std::string& service_name,
                        bool serving) override {
    impl_->SetStatus(service_name, serving ? HealthCheckResponse::SERVING
                                           : HealthCheckResponse::NOT_SERVING);
  }

  void SetServingStatus(bool serving) override {
    impl_->SetAll(serving ? HealthCheckResponse::SERVING
                          : HealthCheckResponse::NOT_SERVING);
  }

  void Shutdown() override { impl_->Shutdown(); }

 private:
  HealthCheckServiceImpl* impl_{nullptr};  // not owned
};

// 注册通用的grpc服务健康监护
void GrpcHealthRegister(const std::string& server_name,
                        ServerBuilder& builder) {
  static nas::HealthCheckServiceImpl health_impl;
  std::unique_ptr<grpc::HealthCheckServiceInterface> override_service(
      new nas::CustomHealthCheckService(&health_impl));
  override_service->SetServingStatus(server_name, true);
  std::unique_ptr<grpc::ServerBuilderOption> option(
      new grpc::HealthCheckServiceServerBuilderOption(
          std::move(override_service)));

  builder.SetOption(std::move(option));
  builder.RegisterService(&health_impl);
}

// 让外部传递服务实现的对象
void GrpcExternHealthRegister(const std::string& server_name,
                        ServerBuilder& builder,
                        nas::HealthCheckServiceImpl* health_impl) {
  std::unique_ptr<grpc::HealthCheckServiceInterface> override_service(
      new nas::CustomHealthCheckService(health_impl));
  override_service->SetServingStatus(server_name, true);
  std::unique_ptr<grpc::ServerBuilderOption> option(
      new grpc::HealthCheckServiceServerBuilderOption(
          std::move(override_service)));

  builder.SetOption(std::move(option));
  builder.RegisterService(health_impl);
}

}  // namespace nas
#endif  // NAS_COMMON_GRPC_HEALTH_CHECK_H