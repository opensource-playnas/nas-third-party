/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-24 15:14:35
 */
#include "grpc_base_service_impl.h"
#include "base/threading/sequence_bound.h"

namespace nas {
GrpcBaseServiceImpl::GrpcBaseServiceImpl(GrpcBaseServiceInterface* server)
    : server_impl_(server), work_thread_("grpc_base_service_thread") {
  if (!work_thread_.IsRunning()) {
    work_thread_.StartWithOptions(
        base::Thread::Options{base::MessagePumpType::IO, 0});
  }
}
GrpcBaseServiceImpl::~GrpcBaseServiceImpl() {
  work_thread_.Stop();
}

Status GrpcBaseServiceImpl::Stop(ServerContext* context,
                                 const StopRequest* request,
                                 StopResponse* response) {
  work_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(
                     [](GrpcBaseServiceInterface* server) {
                       if (server) {
                         server->Stop();
                       }
                     },
                     server_impl_));

  return Status::OK;
}

Status GrpcBaseServiceImpl::Reset(ServerContext* context,
                                  const ResetRequest* request,
                                  ResetResponse* response) {
  int level = request->level();
  base::WaitableEvent wait_event;
  work_thread_.task_runner()->PostTask(
      FROM_HERE, base::BindOnce(
                     [](GrpcBaseServiceInterface* server, int level,
                        base::WaitableEvent& wait_event) {
                       if (server) {
                         server->Reset(level);
                         wait_event.Signal();
                       }
                     },
                     server_impl_, level, std::ref(wait_event)));
  wait_event.Wait();
  return Status::OK;
}
}  // namespace nas
