/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-04-11 16:00:57
 */

#include "nas/common/auth_service_client.h"
#include "base/logging.h"

#include "nas/common/micro_service_info.hpp"
namespace nas {

AuthServiceClient* AuthServiceClient::GetInstance() {
  return base::Singleton<AuthServiceClient>::get();
}

AuthServiceClient::AuthServiceClient() {}
AuthServiceClient::~AuthServiceClient() {}

void AuthServiceClient::Init(
    const std::string& app,
    std::shared_ptr<ServicesMgrStub> services_mgr_stub) {
  services_mgr_stub_ = services_mgr_stub;
  app_ = app;

  CreateAuthServiceStub();

  if (work_thread_) {
    return;
  }

  work_thread_ = std::make_shared<base::Thread>("AuthServiceClient");
  if (work_thread_ && !work_thread_->IsRunning()) {
    work_thread_->StartWithOptions(
        base::Thread::Options{base::MessagePumpType::IO, 0});
  }
}

void AuthServiceClient::UnInit() {
  if (work_thread_) {
    work_thread_->Stop();
  }
}

bool AuthServiceClient::CreateAuthServiceStub() {
  std::string auth_service_address;

  if (services_mgr_stub_) {
    service_manager::v1::GrpcServiceInfo grpc_info;
    services_mgr_stub_->QueryGrpcServices(kAuthServiceName, &grpc_info);
    auth_service_address = grpc_info.ip_port();
  } else if (!address_.empty()) {
    auth_service_address = address_;
  } else {
    LOG(INFO) << "services_mgr_stub and address empty";
    return false;
  }

  LOG(INFO) << "auth_service_address:" << auth_service_address;
  if (!auth_service_address.empty()) {
    std::shared_ptr<grpc::Channel> channel{grpc::CreateChannel(
        auth_service_address, grpc::InsecureChannelCredentials())};

    stub_ = auth_service::v1::AuthService::NewStub(channel);
  } else {
    LOG(INFO) << "auth_service_address: is empty";
  }

  return stub_ != nullptr;
}

bool AuthServiceClient::GetAllUser(auth_service::v1::UserInfoList* user_list) {
  if (!user_list) {
    return false;
  }
  if (!stub_ && !CreateAuthServiceStub()) {
    LOG(ERROR) << "auth service stub is null";
    return false;
  }

  auth_service::v1::EmptyParam request;
  grpc::ClientContext client_context;
  grpc::Status status = stub_->GetAllUser(&client_context, request, user_list);
  if (!status.ok()) {
    LOG(ERROR) << "getalluser error:" << status.error_message();
    // 重置stub_,如果外部需要重试的时候,可以查询新的服务地址
    stub_.reset();
    return false;
  }

  return true;
}

bool AuthServiceClient::GetNasBaseInfo(auth_service::v1::NasBaseInfo* info) {
  if (!info) {
    return false;
  }
  if (!stub_ && !CreateAuthServiceStub()) {
    LOG(ERROR) << "auth service stub is null";
    return false;
  }

  auth_service::v1::EmptyParam request;
  grpc::ClientContext client_context;
  grpc::Status status = stub_->GetNasBaseInfo(&client_context, request, info);
  if (!status.ok()) {
    LOG(ERROR) << "GetNasBaseInfo error:" << status.error_message();
    // 重置stub_,如果外部需要重试的时候,可以查询新的服务地址
    stub_.reset();
    return false;
  }

  return true;
}

bool AuthServiceClient::CleanNasBaseInfo() {
  if (!stub_ && !CreateAuthServiceStub()) {
    LOG(ERROR) << "auth service stub is null";
    return false;
  }

  auth_service::v1::EmptyParam request;
  ::auth_service::v1::OpResult response;
  grpc::ClientContext client_context;
  grpc::Status status =
      stub_->CleanNasBaseInfo(&client_context, request, &response);
  if (!status.ok()) {
    LOG(ERROR) << "CleanNasBaseInfo error:" << status.error_message();
    // 重置stub_,如果外部需要重试的时候,可以查询新的服务地址
    stub_.reset();
    return false;
  }
  return true;
}

bool AuthServiceClient::UpdateNasBaseInfo(auth_service::v1::NasBaseInfo& info) {
  if (!stub_ && !CreateAuthServiceStub()) {
    LOG(ERROR) << "auth service stub is null";
    return false;
  }

  ::auth_service::v1::OpResult response;

  grpc::ClientContext client_context;
  grpc::Status status =
      stub_->UpdateNasBaseInfo(&client_context, info, &response);
  if (!status.ok()) {
    LOG(ERROR) << "UpdateNasBaseInfo error:" << status.error_message();
    // 重置stub_,如果外部需要重试的时候,可以查询新的服务地址
    stub_.reset();
    return false;
  }

  return true;
}

}  // namespace nas