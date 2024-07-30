
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:58

#include "set_screen_casting_task_volume.h"
namespace nas {

void SetScreenCastingTaskVolumeCallData::Process() {
  std::string task_id = request_.task_id();
  int volume = request_.volume();
  LOG(INFO) << "recv SetScreenCastingTaskVolumeCallData ,task_id: " << task_id
            << ", volume: " << volume;
  screen_casting_mgr_->SetTaskVolume(task_id, volume);
  OnProcessed();
}

void SetScreenCastingTaskVolumeCallData::ProcessComplete() {}

}  // namespace nas
