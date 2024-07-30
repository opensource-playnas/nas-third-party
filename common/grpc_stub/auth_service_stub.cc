/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 15:16:52
 */

#include "auth_service_stub.h"

#include "base/logging.h"

#include "google/protobuf/message.h"
#include "nas/common/error_code/general_error_code.h"
#include "nas/common/micro_service_info.hpp"

#include "grpc_client_context.hpp"

namespace nas {

AuthServiceStub::AuthServiceStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kAuthServiceName;
}

void AuthServiceStub::Dispatch(const std::string& param,
                               AsyncGrpcResponseHandler grpc_response_handler) {
  google::protobuf::util::JsonParseOptions options;
  options.ignore_unknown_fields = true;

  bool processed = true;
  switch (handler_id_) {
    case HandlerRequestId::kDeviceBind:
      (new AsyncClientCall<auth_service::v1::OpResult,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncDeviceBind(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kExchangeBind:
      (new AsyncClientCall<auth_service::v1::OpResult,
                           auth_service::v1::BindUpdateRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::BindUpdateRequest& request) {
             return auth_service_stub_->PrepareAsyncDeviceBindUpdate(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kRetsetDeviveBind:
      (new AsyncClientCall<auth_service::v1::OpResult,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncResetNasSetting(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kCodeLogin:
      (new AsyncClientCall<auth_service::v1::LoginInfo,
                           auth_service::v1::LoginCode>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::LoginCode& request) {
             return auth_service_stub_->PrepareAsyncJumpLogin(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kAccessTokenLogin:
      (new AsyncClientCall<auth_service::v1::LoginInfo,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncSecondLogin(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kLogout:
      (new AsyncClientCall<auth_service::v1::OpResult,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncLogout(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetAllTokens:
      (new AsyncClientCall<auth_service::v1::VerifyTokensResponse,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncGetLoginToken(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kVerifyRequest:
      (new AsyncClientCall<auth_service::v1::VerifyTokenResponse,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncVerifyToken(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kAddNasUserInfo:
      (new AsyncClientCall<auth_service::v1::OpResult,
                           auth_service::v1::UserInfoList>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::UserInfoList& request) {
             return auth_service_stub_->PrepareAsyncAddNasUserInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetNasBaseInfo:
      (new AsyncClientCall<auth_service::v1::NasBaseInfo,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncGetNasBaseInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetUserPayPlan:
      (new AsyncClientCall<auth_service::v1::ProxyCenterResponse,
                           auth_service::v1::RequestPayList>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::RequestPayList& request) {
             return auth_service_stub_->PrepareAsyncGetUserPayPlan(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kSelectPayPlan:
      (new AsyncClientCall<auth_service::v1::ProxyCenterResponse,
                           auth_service::v1::RequestSelectPlanId>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::RequestSelectPlanId& request) {
             return auth_service_stub_->PrepareAsyncSelectPayPlan(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetCurPayPlan:
      (new AsyncClientCall<auth_service::v1::ProxyCenterResponse,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncGetCurPayPlan(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    case HandlerRequestId::kGetCurUserInfo:
      (new AsyncClientCall<auth_service::v1::UserInfo,
                           auth_service::v1::EmptyParam>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const auth_service::v1::EmptyParam& request) {
             return auth_service_stub_->PrepareAsyncGetUserInfo(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;
    default:
      processed = false;
      break;
  }
  DCHECK(processed);
}

bool AuthServiceStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);

  AsyncInitStub(base::BindOnce(&AuthServiceStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void AuthServiceStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](AuthServiceStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->auth_service_stub_ =
                auth_service::v1::AuthService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}

}  // namespace nas