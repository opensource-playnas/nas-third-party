/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-09-06 13:38:05
 */
#ifndef NAS_COMMON_GRPC_STUB_GRPC_STUB_ENV_H_
#define NAS_COMMON_GRPC_STUB_GRPC_STUB_ENV_H_

#include <memory>
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"

#include "grpcpp/channel.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/impl/codegen/channel_interface.h"
#include "grpcpp/impl/codegen/completion_queue.h"

#include "nas/common/service_manager/services_mgr_stub.h"
namespace nas {
/*
提供grpc 发起异步请求的环境,使用的时候需要确保所有可能
用到grpc 完成队列的线程退出后再进行反初始化操作
*/
using GrpcChannelPtr = std::shared_ptr<grpc::Channel>;
using GetChannelCallback = base::OnceCallback<void(GrpcChannelPtr)>;
using IpPortCallback = base::OnceCallback<void(const std::string&)>;
using GetServiceIpPortCallback =
    base::RepeatingCallback<void(const std::string& service_name,
                                 IpPortCallback callback)>;
class GrpcStubEnv {
 public:
  static GrpcStubEnv* GetInstance();

  void GetUsableChannel(const std::string& service_name,
                        GetChannelCallback channel_callback);
  grpc::CompletionQueue* GetCompletionQueue();
  void Start(GetServiceIpPortCallback ip_port_callback);
  void End();

  // 由于希望模块内部都复用同一个service_mgr_stub,提供存取接口
  void SetServiceMgrStub(std::shared_ptr<ServicesMgrStub> service_mgr_stub);
  std::shared_ptr<ServicesMgrStub> GetServiceMgrStub();

 private:
  friend struct base::DefaultSingletonTraits<GrpcStubEnv>;
  GrpcStubEnv();
  virtual ~GrpcStubEnv();
  void CompleteGrpc();
  void ShutdownCompletionQueue();

  void AddChannel(const std::string& service_name, GrpcChannelPtr channel);
  void RemoveChannel(const std::string& service_name);
  GrpcChannelPtr GetChannel(const std::string& service_name);
  void AsyncCreateChannelAndRegister(const std::string& service_name,
                                     GetChannelCallback create_callback);

  /*
   * 对channel 注册channel的通知,当channel发生中断时及时重连。
   * 如果Channel处于Ready状态，不关心超时;
   * 如果Channel处于Connecting状态,也不超时,等待状态继续流转;
   * 如果Channel处于Idle或Failed，重新获取端口创建,避免端口改变无法连接上
   */
  void RegisterOnStateChange(const std::string& service_name,
                             GrpcChannelPtr channel,
                             bool is_first);
  void StateChange(const std::string& service_name, GrpcChannelPtr channel);

 private:
  std::unique_ptr<base::Thread> grpc_thread_;
  // 实现grpc 接口的异步访问
  base::Lock shutdown_lock_;
  std::unique_ptr<grpc::CompletionQueue> completeion_queue_;
  GetServiceIpPortCallback get_service_ip_port_callback_;

  std::shared_ptr<ServicesMgrStub> service_mgr_stub_;
  using ChannelMgr = std::map<std::string, GrpcChannelPtr>;
  ChannelMgr channel_mgr_;
  base::Lock channel_mgr_lock_;
  std::atomic<bool> is_stoped_ = false;
};  // class GrpcStubMgr
}  // namespace nas

#endif  // NAS_COMMON_GRPC_STUB_GRPC_STUB_ENV_H_