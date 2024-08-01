/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-20 16:04:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_BT_TORRENT_TASK_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_BT_TORRENT_TASK_H_

#include "bt_task_base.h"

namespace nas {
class BtTorrentTask : public BtTaskBase {
 public:
  BtTorrentTask(TaskParamPtr param, const ContextField& context_field);
  bool Parse() override;
  bool ExceCancel(bool is_delete_local_file, bool real_delete) override;
  void CreateMagnetUri() override;

 private:
  void DeleteTorrentFile();
};
using BtTorrentTaskPtr = std::shared_ptr<BtTorrentTask>;
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_BT_TORRENT_TASK_H_