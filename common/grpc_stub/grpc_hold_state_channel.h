/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-07-21 11:09:38
 */

#ifndef NAS_COMMON_GRPC_STUB_GRPC_HOLD_STATE_CHANNEL_H_
#define NAS_COMMON_GRPC_STUB_GRPC_HOLD_STATE_CHANNEL_H_
#include <map>
#include <memory>
#include "base/check.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"

#include "grpcpp/grpcpp.h"
#include "grpcpp/channel.h"

#include "grpc_stub_env.h"


namespace nas {
// 这个类尽量让grpc channel的连接状态处于ready状态
class GrpcHoldStateChannel {
 public:
  GrpcHoldStateChannel(GrpcStubEnv* stub_env);

  // 获取Channel自动监控状态
  void GetChannel(const std::string& service_name, GetChannelCallback channel_callback);
 private:
  // 放在这里方便管理channel,连接状态
  GrpcChannelPtr channel_;
  std::string service_name_;
  GrpcStubEnv* stub_env_;
};  // class GrpcChannelState
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_GRPC_HOLD_STATE_CHANNEL_H_