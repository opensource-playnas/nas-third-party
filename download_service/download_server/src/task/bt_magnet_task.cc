/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-20 16:04:06
 */

#include "bt_magnet_task.h"
#include "nas/common/call_result.h"
#include "nas/common/path/file_path_unit.h"
#include "utils/utils.h"

namespace nas {
BtMagnetTask::BtMagnetTask(TaskParamPtr param,
                           const ContextField& context_field)
    : BtTaskBase(param, context_field) {}

bool BtMagnetTask::Parse() {
  bool ret = false;
  MagnetDownloadTask* impl = static_cast<MagnetDownloadTask*>(task_impl_);
  if (!impl) {
    return ret;
  }
  // 先直接解析一遍，大部分情况是拿不到magnet链接内容的，需要设置回调，等元数据加载到时回调通知
  if (!ParseTorrent()) {
    SetTaskStatus(TaskStatus::kParsing);
    impl->SetParseCallback(this, &BtMagnetTask::ParseMangetResult);
  } else {
    GetParseCompletedCallback().Run(context_field_.user_id(),
                                    GetTaskParam()->id);
    ret = true;
  }
  return ret;
}

void BtMagnetTask::SetParseCompletedCallback(
    OnParseCompletedCallback callback) {
  parse_callback_ = std::move(callback);
}

OnParseCompletedCallback BtMagnetTask::GetParseCompletedCallback() {
  return std::move(parse_callback_);
}

void BtMagnetTask::UpdateFileBaseInfo(const std::string& result) {
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  task_base_info->SetFileList(result);

  FileBaseInfo base_info = task_impl_->GetTorrentFileBaseInfo();
  task_base_info->SetTaskName(base_info.file_name);
  task_base_info->SetTaskLandFileName(base_info.file_name);
  task_base_info->SetTaskFileSize(base_info.file_size);
  task_base_info->SetTaskFileType(
      std::to_string(nas::path_unit::FileType::kBt));
}

void BtMagnetTask::ParseMangetResult(void* context, const char* result) {
  LOG(INFO) << "parse magnet completed!";
  BtMagnetTask* task = (BtMagnetTask*)context;
  if (!task) {
    return;
  }

  task->GetParseCompletedCallback().Run(task->GetTaskParam()->id, result);
}

void BtMagnetTask::CreateMagnetUri() {
  TaskParamPtr task_param = task_info_->GetTaskParam();
  if (task_param) {
    task_param->converted_url = task_param->url;
  }
}

}  // namespace nas
