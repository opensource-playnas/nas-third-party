// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:37

#include "play_screen_casting_task.h"

namespace nas {

void PlayScreenCastingTaskCallDate::Process() {
  std::string task_id = request_.task_id();
  LOG(INFO) << "recv PlayScreenCastingTaskCallDate ,task_id: " << task_id;

  screen_casting_mgr_->PlayTask(task_id);

  OnProcessed();
}

void PlayScreenCastingTaskCallDate::ProcessComplete() {}

}  // namespace nas
