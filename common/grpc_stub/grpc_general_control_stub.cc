/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-09-21 20:25:03
 */

#include "grpc_general_control_stub.h"

#include "nas/common/grpc_stub/request_handler_id.hpp"

namespace nas {
GrpcGeneralControlStub::GrpcGeneralControlStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header,
    const std::string& service_name)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = service_name;
}

void GrpcGeneralControlStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kResetGrpcService:
      (new AsyncClientCall<ResetResponse,
                           ResetRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const ResetRequest& request) {
            grpc::ClientContext& context = client_context_->ClientContext();
            // 设置超时时间
            std::chrono::system_clock::time_point deadline =
                std::chrono::system_clock::now() + std::chrono::seconds(30);
            context.set_deadline(deadline);
             return grpc_base_stub_->PrepareAsyncReset(
                 &context, request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kStopGrpcService:
      (new AsyncClientCall<StopResponse,
                           StopRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const StopRequest& request) {
             return grpc_base_stub_->PrepareAsyncStop(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    default: {
      processed = false;
      LOG(ERROR) << "error branch handler_id_:" << static_cast<int>(handler_id_);
    }
  }
  DCHECK(processed);
}

bool GrpcGeneralControlStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  
  AsyncInitStub(base::BindOnce(&GrpcGeneralControlStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void GrpcGeneralControlStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](GrpcGeneralControlStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->grpc_base_stub_ =
                GrpcBaseService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas