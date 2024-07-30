// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:35

#include "cancel_screen_casting_task.h"

namespace nas {

void CancelScreenCastingTaskCallDate::Process() {
  std::string task_id = request_.task_id();
  LOG(INFO) << "recv CancelScreenCastingTaskCallDate ,task_id: " << task_id;

  if (!task_id.empty()) {
    screen_casting_mgr_->CancelTask(task_id);
  } else {
    call_result_.errcode = (int)DlnaServiceErrorNo::kParamInvalid;
  }
  OnProcessed();
}

void CancelScreenCastingTaskCallDate::ProcessComplete() {}

}  // namespace nas
