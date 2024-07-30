// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/07 19:47

#include "change_screen_casting_task_play_url.h"

namespace nas {

void ChangeScreenCastingTaskPlayUrlCallDate::Process() {
  std::string task_id = request_.task_id();
  std::string play_url = request_.play_url();
  LOG(INFO) << "recv ChangeScreenCastingTaskPlayUrlCallDate ,task_id: "
            << task_id << ", play url: " << play_url;

  screen_casting_mgr_->ChangeTaskPlayUrl(task_id, play_url);
  OnProcessed();
}

void ChangeScreenCastingTaskPlayUrlCallDate::ProcessComplete() {}

}  // namespace nas
