/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-20 16:04:06
 */

#include "bt_torrent_task.h"

namespace nas {
BtTorrentTask::BtTorrentTask(TaskParamPtr param,
                             const ContextField& context_field)
    : BtTaskBase(param, context_field) {}

bool BtTorrentTask::Parse() {
  if (!task_impl_) {
    return false;
  }
  return true;
}

void BtTorrentTask::CreateMagnetUri() {
  TorrentFileDownloadTask* impl =
      static_cast<TorrentFileDownloadTask*>(task_impl_);
  if (!impl || !converted_source_.empty()) {
    return;
  }

  int size = impl->CreateMagnetUri(NULL);
  if (size <= 0) {
    return;
  }
  char* buffer = new char[size + 1];
  memset(buffer, 0, sizeof(char) * (size + 1));
  impl->CreateMagnetUri(buffer);
  TaskParamPtr task_param = task_info_->GetTaskParam();
  if (task_param) {
    converted_source_ = buffer;
    task_param->converted_url = converted_source_;
  }
  if (buffer) {
    delete[] buffer;
    buffer = NULL;
  }
}

bool BtTorrentTask::ExceCancel(bool is_delete_local_file, bool real_delete) {
  bool ret = true;
  if (real_delete) {
    if (GetDownloadTaskImp()->Cancel(is_delete_local_file)) {
      SetTaskStatus(TaskStatus::kCompletedDeleted);
      DeleteTorrentFile();
    } else {
      ret = false;
    }
  }

  return ret;
}

void BtTorrentTask::DeleteTorrentFile() {
  TorrentTaskParamPtr torrent_param =
      std::static_pointer_cast<TorrentTaskParam>(task_info_->GetTaskParam());
  if (torrent_param) {
    nas::utils::DeleteFile(torrent_param->url);
    if (!torrent_param->is_private_cloud_source) {
      nas::utils::DeleteFile(torrent_param->front_trans_url);
    }
  }
}

}  // namespace nas