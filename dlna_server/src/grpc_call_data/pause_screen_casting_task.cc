
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:50
#include "pause_screen_casting_task.h"

namespace nas {

void PauseScreenCastingTaskCallData::Process() {
  std::string task_id = request_.task_id();
  LOG(INFO) << "recv PauseScreenCastingTaskCallData ,task_id: " << task_id;
  screen_casting_mgr_->PauseTask(task_id);

  OnProcessed();
}

void PauseScreenCastingTaskCallData::ProcessComplete() {}

}  // namespace nas
