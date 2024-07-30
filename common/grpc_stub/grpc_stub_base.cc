/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-02-01 11:20:44
 */

#include "grpc_stub_base.h"

#include "base/json/json_writer.h"
#include "base/logging.h"

#include "google/protobuf/message.h"
#include "google/protobuf/util/json_util.h"

#include "grpc_client_context.hpp"
#include "grpc_stub_env.h"

namespace nas {

GrpcStubBase::GrpcStubBase(
    HandlerRequestId handler_id,
    const std::multimap<std::string, std::string>& header) {
  handler_id_ = handler_id;
  request_header_ = header;
  state_channel_ =
      std::make_shared<GrpcHoldStateChannel>(GrpcStubEnv::GetInstance());
  client_context_ = std::make_shared<GrpcClientContext>();
}

GrpcStubBase::~GrpcStubBase() {}

void GrpcStubBase::InitCompletionQueue(
    grpc::CompletionQueue* grpc_completion_queue) {
  grpc_completion_queue_ = grpc_completion_queue;
}

void GrpcStubBase::AsyncInitStub(GetStubCallbackCallback){

}
bool GrpcStubBase::Handler(const std::string& param) {
  client_context_ = std::make_shared<GrpcClientContext>();
  for (auto& iter : request_header_) {
    client_context_->ClientContext().AddMetadata(iter.first, iter.second);
  }
  return true;
}

}  // namespace nas