/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-07-31 10:53:43
 */

#include "grpc_hold_state_channel.h"

#include "grpc_async_call_base.h"
#include "grpc_client_context.hpp"
namespace nas {

GrpcHoldStateChannel::GrpcHoldStateChannel(GrpcStubEnv* stub_env) {
  stub_env_ = stub_env;
}

void GrpcHoldStateChannel::GetChannel(const std::string& service_name,
                                      GetChannelCallback channel_callback) {
  service_name_ = service_name;
  if (!channel_) {
    stub_env_->GetUsableChannel(service_name_, std::move(channel_callback));
  } else {
    std::move(channel_callback).Run(channel_);
  }
}

}  // namespace nas