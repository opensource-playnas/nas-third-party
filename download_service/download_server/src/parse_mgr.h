/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-04-18 19:57:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_PARSE_MGR_H_
#define NAS_DOWNLOAD_SERVER_SRC_PARSE_MGR_H_

#include <memory>

#include "base/threading/thread.h"
#include "base/timer/timer.h"
#include "nas/common/nas_thread.h"
#include "nas/common/context_field.h"
#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"
#include "nas/download_service/third_party/download_public_define.h"
#include "task/task_base.h"

namespace nas {
class ParseManager : public std::enable_shared_from_this<ParseManager> {
 public:
  ParseManager();
  ~ParseManager();


DownloadErrorCode ParseLink(
    const download::v1::ParseLinkRequest* request,
    download::v1::ParseLinkReply* response,
    const ContextField& context_field);

  DownloadErrorCode ParseTorrent(
      const download::v1::ParseTorrentRequest* request,
      download::v1::ParseTorrentReply* response,
      const ContextField& context_field);
private:
    std::shared_ptr<NasThread> parse_thread_ = nullptr;
};
using ParseManagerPtr = std::shared_ptr<ParseManager>;
}  // namespace nas
#endif