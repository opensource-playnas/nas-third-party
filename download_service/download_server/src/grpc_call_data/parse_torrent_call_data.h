/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-07-12 20:01:41
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_PARSE_TORRENT_CALL_DATA_H
#define NAS_DOWNLOAD_SERVER_SRC_PARSE_TORRENT_CALL_DATA_H

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
using download::v1::ParseTorrentRequest;
using download::v1::ParseTorrentReply;
using download::v1::DownloadService;

class ParseTorrentCallData
    : public nas::GrpcServerCallBase<nas::DownloadServiceImpl,
                                         ParseTorrentCallData> {
 public:
  ParseTorrentCallData(DownloadServiceImpl* service,
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

  ParseTorrentRequest request_;
  ParseTorrentReply response_;

  grpc::ServerAsyncResponseWriter<ParseTorrentReply> responder_;
  nas::CallResult call_request_;
};

};  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_PARSE_TORRENT_CALL_DATA_H