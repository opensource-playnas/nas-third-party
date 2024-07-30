/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 19:32:06
 */
#include "message_center_stub.h"

#include "base/logging.h"
#include "base/task/thread_pool.h"

#include "nas/common/micro_service_info.hpp"

#include "grpc_client_context.hpp"

namespace nas {

MessageCenterStub::MessageCenterStub(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header)
    : GrpcStubBase(handler_id, header) {
  service_package_name_ = kMessageCenterName;
}

MessageCenterStub::~MessageCenterStub() {
}

void MessageCenterStub::Dispatch(const std::string& param,
                AsyncGrpcResponseHandler grpc_response_handler){

  bool processed = true;
  switch(handler_id_) {
    case HandlerRequestId::kMessageHistory:
      (new AsyncClientCall<messages_center::GetMessageDetailResopnse,
                           messages_center::GetMessageDetailRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const messages_center::GetMessageDetailRequest& request) {
             return message_center_stub_->PrepareAsyncGetMessageDetail(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break;  
    case HandlerRequestId::kMessageDelId:
      (new AsyncClientCall<messages_center::MessageResponse,
                           messages_center::MessageIds>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const messages_center::MessageIds& request) {
             return message_center_stub_->PrepareAsyncDelMessageByIds(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kMessageDelType:
      (new AsyncClientCall<messages_center::MessageResponse,
                           messages_center::DelMessageByTypeRequest>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const messages_center::DelMessageByTypeRequest& request) {
             return message_center_stub_->PrepareAsyncDelMessageByType(
                 &client_context_->ClientContext(), request,
                 grpc_completion_queue_);
           }))
          ->Call(param);
      break; 
    case HandlerRequestId::kMessageAdd:
      (new AsyncClientCall<messages_center::MessageResponse,
                           messages_center::UserMessage>(
           std::move(grpc_response_handler), handler_id_, client_context_,
           [this](const messages_center::UserMessage& request) {
             return message_center_stub_->PrepareAsyncPublishToMessageCenter(
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
bool MessageCenterStub::AsyncHandler(
    const std::string& param,
    AsyncGrpcResponseHandler grpc_response_handler) {
  GrpcStubBase::Handler(param);
 
  AsyncInitStub(base::BindOnce(&MessageCenterStub::Dispatch,
                               base::RetainedRef(this), param,
                               std::move(grpc_response_handler)));
  return true;
}

void MessageCenterStub::AsyncInitStub(GetStubCallbackCallback callback) {
  state_channel_->GetChannel(
      service_package_name_,
      base::BindOnce(
          [](MessageCenterStub* stub, GetStubCallbackCallback callback,
             GrpcChannelPtr channel) {
            if (!channel) {
              LOG(ERROR) << "get channel failed,service name:"
                         << stub->service_package_name_;
              return;
            }
            stub->message_center_stub_ = messages_center::MessageCenter::NewStub(channel);
            std::move(callback).Run();
          },
          base::Unretained(this), std::move(callback)));
}
}  // namespace nas