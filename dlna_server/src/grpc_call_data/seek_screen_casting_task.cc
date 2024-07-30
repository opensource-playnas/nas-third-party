
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:41

#include "seek_screen_casting_task.h"

namespace nas {

void SeekScreenCastingTaskCallDate::Process() {
  std::string task_id = request_.task_id();
  int64_t  position = request_.position();
  LOG(INFO) << "recv SeekScreenCastingTaskCallDate ,task_id: " << task_id
            << ", posititon: " << position;
  screen_casting_mgr_->SeekTask(task_id, position);
  OnProcessed();
}

void SeekScreenCastingTaskCallDate::ProcessComplete() {}

}  // namespace nas
