/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-07-12 20:01:41
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_CLEAN_CALL_DATA_H
#define NAS_DOWNLOAD_SERVER_SRC_CLEAN_CALL_DATA_H

#include "base/task/sequenced_task_runner.h"

#include "nas/common/call_result.h"
#include "nas/common/context_field.h"
#include "nas/common/grpc_server_call_data.h"
#include "nas/common/nas_thread.h"
#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"
#include "nas/download_service/download_server/src/download_grpc_server.h"

namespace nas {

using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using download::v1::CleanRequest;
using download::v1::CleanReply;
using download::v1::DownloadService;

class CleanCallData
    : public nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                                         CleanCallData> {
 public:
  CleanCallData(DownloadServiceImpl* service,
                            ServerCompletionQueue* cq,
                            NasThread* cq_dispatch,
                            NasThread* task_main);

 protected:
  void Request(DownloadServiceImpl* service,
               ServerCompletionQueue* cq) override;
  void Process() override;
  void Finish() override;

 private:
  ServerContext ctx_;
  CallResult call_result_;

  CleanRequest request_;
  CleanReply response_;

  grpc::ServerAsyncResponseWriter<CleanReply> responder_;
  nas::CallResult call_request_;
};

};  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_CLEAN_CALL_DATA_H