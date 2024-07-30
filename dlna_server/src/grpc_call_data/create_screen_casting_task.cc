
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/09/04 15:31

#include "create_screen_casting_task.h"
namespace nas {

void CreateScreenCastingTaskCallDate::Process() {
  std::string device_id = request_.device_id();
  std::string play_url = request_.play_url();
  ContextField grpc_context_field(&ctx_);

  LOG(INFO) << "recv CreateScreenCastingTaskCallDate ,play_url: " << play_url
            << ", device_id: " << device_id
            << ", X-Endpoint-Id: " << grpc_context_field.x_endpoint_id();

  if (play_url.empty() || device_id.empty() ||
      grpc_context_field.x_endpoint_id().empty()) {
    call_result_.errcode = (int)DlnaServiceErrorNo::kParamInvalid;
    OnProcessed();
    return;
  }

  screen_casting_mgr_->CreateTask(
      device_id, play_url, grpc_context_field,
      base::BindOnce(&CreateScreenCastingTaskCallDate::ProcessComplete,
                     base::Unretained(this)));
}

void CreateScreenCastingTaskCallDate::ProcessComplete(
    const std::string& task_id,
    DlnaServiceErrorNo err_code) {

  if (task_id.empty()) {
    call_result_.errcode = (int)err_code;
  }
  response_.set_task_id(task_id);
  OnProcessed();
}

}  // namespace nas
