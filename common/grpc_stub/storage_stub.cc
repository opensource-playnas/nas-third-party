#include "storage_stub.h"

#include "grpc_client_context.hpp"
#include "nas/common/micro_service_info.hpp"

namespace nas {
StorageServiceStub::StorageServiceStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kSystemInfoServiceName;
}

void StorageServiceStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kPutStorage:
      (new AsyncClientCall<storage::v1::PutStorageResponse,
                           storage::v1::PutStorageRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const storage::v1::PutStorageRequest& request) {
             return storage_service_stub_->PrepareAsyncPutStorage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kGetStorage:
      (new AsyncClientCall<storage::v1::GetStorageResponse,
                           storage::v1::GetStorageRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const storage::v1::GetStorageRequest& request) {
             return storage_service_stub_->PrepareAsyncGetStorage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kListStorage:
      (new AsyncClientCall<storage::v1::ListStorageResponse,
                           storage::v1::ListStorageRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const storage::v1::ListStorageRequest& request) {
             return storage_service_stub_->PrepareAsyncListStorage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kResetStorage:
      (new AsyncClientCall<storage::v1::ResetStorageResponse,
                           storage::v1::ResetStorageRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const storage::v1::ResetStorageRequest& request) {
             return storage_service_stub_->PrepareAsyncResetStorage(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;   
    default: {
      processed = false;
      LOG(ERROR) << "error branch";
    }
  }
  DCHECK(processed);
}

bool StorageServiceStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
  
  AsyncInitStub(base::BindOnce(&StorageServiceStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void StorageServiceStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](StorageServiceStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->storage_service_stub_ = storage::v1::StorageService::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas