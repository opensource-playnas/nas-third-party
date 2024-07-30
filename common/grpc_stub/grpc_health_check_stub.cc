/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-12-26 19:40:47
*/
#include "grpc_health_check_stub.h"

#include "nas/common/grpc_stub/request_handler_id.hpp"

namespace nas {
GrpcHealthCheckStub::GrpcHealthCheckStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header,
    const std::string& service_name)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = service_name;
}

void GrpcHealthCheckStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kHealthCheck:
      (new AsyncClientCall<grpc::health::v1::HealthCheckResponse,
                           grpc::health::v1::HealthCheckRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const grpc::health::v1::HealthCheckRequest& request) {
            grpc::ClientContext& context = client_context_->ClientContext();
             return hc_stub_->PrepareAsyncCheck(
                 &context, request,
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

bool GrpcHealthCheckStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  
  AsyncInitStub(base::BindOnce(&GrpcHealthCheckStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void GrpcHealthCheckStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](GrpcHealthCheckStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->hc_stub_ =
                grpc::health::v1::Health::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas