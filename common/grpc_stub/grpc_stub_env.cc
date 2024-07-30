/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-09-06 16:08:58
 */

#include "grpc_stub_env.h"
#include "base/logging.h"

#include "grpcpp/impl/codegen/channel_interface.h"

#include "grpc_async_call_base.h"
#include "grpc_client_context.hpp"

namespace nas {

using ChannelStateDelegate = base::RepeatingCallback<void()>;
class ChannelStateCheckCall : public AsyncClientCallBase {
 public:
  ChannelStateCheckCall(std::shared_ptr<GrpcClientContext> context,
                        ChannelStateDelegate channel_state_handler)
      : AsyncClientCallBase(context),
        channel_state_handler_(std::move(channel_state_handler)) {}

  virtual void HandleResponse() { channel_state_handler_.Run(); }

 private:
  ChannelStateDelegate channel_state_handler_;
};  // ChannelStateCheckCall

GrpcStubEnv::GrpcStubEnv() {}

GrpcStubEnv::~GrpcStubEnv() {}

GrpcStubEnv* GrpcStubEnv::GetInstance() {
  return base::Singleton<GrpcStubEnv>::get();
}

grpc::CompletionQueue* GrpcStubEnv::GetCompletionQueue() {
  return completeion_queue_.get();
}

void GrpcStubEnv::Start(GetServiceIpPortCallback ip_port_callback) {
  get_service_ip_port_callback_ = std::move(ip_port_callback);
  completeion_queue_ = std::make_unique<grpc::CompletionQueue>();
  grpc_thread_ = std::make_unique<base::Thread>("grpc_async_response");
  if (grpc_thread_) {
    grpc_thread_->Start();
    grpc_thread_->task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&GrpcStubEnv::CompleteGrpc, base::Unretained(this)));
  }
}

void GrpcStubEnv::SetServiceMgrStub(
    std::shared_ptr<ServicesMgrStub> service_mgr_stub) {
  service_mgr_stub_ = service_mgr_stub;
}
std::shared_ptr<ServicesMgrStub> GrpcStubEnv::GetServiceMgrStub() {
  return service_mgr_stub_;
}

void GrpcStubEnv::ShutdownCompletionQueue() {
  base::AutoLock lock(shutdown_lock_);
  completeion_queue_->Shutdown();
}

void GrpcStubEnv::End() {
  is_stoped_ = true;
  ShutdownCompletionQueue();
  if (grpc_thread_) {
    if (grpc_thread_->IsRunning()) {
      grpc_thread_->Stop();
    }
  }
  channel_mgr_.clear();
  get_service_ip_port_callback_.Reset();
}

void GrpcStubEnv::GetUsableChannel(const std::string& service_name,
                                   GetChannelCallback channel_callback) {
  GrpcChannelPtr channel = GetChannel(service_name);
  if (!channel) {
    AsyncCreateChannelAndRegister(service_name, std::move(channel_callback));
  } else {
    std::move(channel_callback).Run(channel);
  }
}

void GrpcStubEnv::AddChannel(const std::string& service_name,
                             GrpcChannelPtr channel) {
  base::AutoLock lock(channel_mgr_lock_);
  auto iter = channel_mgr_.find(service_name);
  if (iter != channel_mgr_.end()) {
    if (iter->second) {
      iter->second.reset();
    }
    iter->second = channel;
  } else {
    channel_mgr_.insert(std::make_pair(service_name, channel));
  }
}

void GrpcStubEnv::RemoveChannel(const std::string& service_name) {
  base::AutoLock lock(channel_mgr_lock_);
  channel_mgr_.erase(service_name);
}

GrpcChannelPtr GrpcStubEnv::GetChannel(const std::string& service_name) {
  base::AutoLock lock(channel_mgr_lock_);
  auto iter = channel_mgr_.find(service_name);
  GrpcChannelPtr channel_ptr;
  if (iter != channel_mgr_.end()) {
    channel_ptr = iter->second;
  }
  return channel_ptr;
}

void GrpcStubEnv::AsyncCreateChannelAndRegister(
    const std::string& service_name,
    GetChannelCallback create_callback) {
  DCHECK(!service_name.empty());
  if (!get_service_ip_port_callback_.is_null()) {
    get_service_ip_port_callback_.Run(
        service_name,
        base::BindOnce(
            [](GrpcStubEnv* grpc_stub_env, const std::string& service_name,
               GetChannelCallback create_callback, const std::string& ip_port) {
              GrpcChannelPtr channel = grpc::CreateChannel(
                  ip_port, grpc::InsecureChannelCredentials());
              grpc_stub_env->AddChannel(service_name, channel);

              grpc_stub_env->RegisterOnStateChange(service_name, channel, true);
              std::move(create_callback).Run(channel);
            },
            base::Unretained(this), service_name, std::move(create_callback)));
  } else {
    std::move(create_callback).Run(nullptr);
  }
}

void GrpcStubEnv::RegisterOnStateChange(const std::string& service_name,
                                        GrpcChannelPtr channel,
                                        bool is_first) {
  if (channel && !is_stoped_) {
    grpc_connectivity_state last_state = channel->GetState(is_first);

    gpr_timespec deadline{};
    // 默认超时检测时间为1s
    deadline.tv_sec = 1;
    deadline.clock_type = GPR_TIMESPAN;
    if (last_state == grpc_connectivity_state::GRPC_CHANNEL_READY) {
      // 如果已经是ready状态的不用超时检测
      deadline.tv_sec = INT64_MAX;
    }

    std::shared_ptr<nas::GrpcClientContext> context;
    ChannelStateCheckCall* call = new ChannelStateCheckCall(
        context,
        base::BindRepeating(&GrpcStubEnv::StateChange, base::Unretained(this),
                            service_name, channel));

    // 完成队列未关闭才添加任务
    base::AutoLock lock(shutdown_lock_);
    if (!is_stoped_) {
      channel->NotifyOnStateChange(last_state, deadline,
                                   completeion_queue_.get(), (void*)call);
    }
  }
}

void GrpcStubEnv::StateChange(const std::string& service_name,
                              GrpcChannelPtr channel) {
  DCHECK(channel);
  grpc_connectivity_state state = GRPC_CHANNEL_IDLE;
  if (channel) {
    state = channel->GetState(false);
  }
  switch (state) {
    case grpc_connectivity_state::GRPC_CHANNEL_CONNECTING:
    case grpc_connectivity_state::GRPC_CHANNEL_READY:
      RegisterOnStateChange(service_name, channel, false);
      break;
    case grpc_connectivity_state::GRPC_CHANNEL_IDLE:
    case grpc_connectivity_state::GRPC_CHANNEL_TRANSIENT_FAILURE: {
      if (is_stoped_) {
        break;
      }

      // 失败的channel重新创建，因为可能是由于端口号也不对
      AsyncCreateChannelAndRegister(
          service_name,
          base::BindOnce(
              [](const std::string& service_name, GrpcChannelPtr channel) {
                if (!channel) {
                  LOG(WARNING)
                      << "service:" << service_name << "reconnect failed";
                }
              },
              service_name));
      break;
    }
    case grpc_connectivity_state::GRPC_CHANNEL_SHUTDOWN: {
      LOG(WARNING) << " grpc channel shutdown";
    }
  }
}

void GrpcStubEnv::CompleteGrpc() {
  void* got_tag = nullptr;
  bool ok = false;

  // Block until the next result is available in the completion queue "cq".
  while (completeion_queue_->Next(&got_tag, &ok)) {
    AsyncClientCallBase* call = static_cast<AsyncClientCallBase*>(got_tag);

    call->HandleResponse();
    if (!ok) {
      LOG(ERROR) << "grpc next not ok";
    }

    // Once we're complete, deallocate the call object.
    delete call;
  }
}

}  // namespace nas