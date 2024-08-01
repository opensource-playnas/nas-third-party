/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#include "mule_task.h"

namespace nas {

MuleTask::MuleTask(TaskParamPtr param, const ContextField& context_field)
    : LinkTask(param, context_field) {
  if (!task_impl_) {
    task_impl_ =
        DownloadInterfaceHelper::GetInstance()->CreateMuleDownloadTask();
  }
  SetDownloadParam(param);
  param->hash = GetTaskHash();
}

MuleTask::~MuleTask() {
  if (task_impl_) {
    DownloadInterfaceHelper::GetInstance()->ReleaseMuleTask(task_impl_);
    task_impl_ = NULL;
  }
}

int MuleTask::SyncParse(ParseResult* result) {
  // 解析ed2k链接
  bool ret = task_impl_->ParseLink(GetTaskParam()->url.c_str(), result);
  if (!ret) {
    return 1;
  }
  return 0;
}

DownloadErrorCode MuleTask::CheckCondition() {
  if (!task_impl_) {
    return DownloadErrorCode::kCreateTaskFailed;
  }
  return task_impl_->CheckCondition(GetTaskParam()->url.c_str());
}

void MuleTask::SetDownloadParam(const TaskParamPtr param) {
  DownloadParam download_param;
  AssembleDownloadParam(&download_param, param);
  task_impl_->SetParam(download_param);
} 

DownloadTaskInterface * MuleTask::GetDownloadTaskImp() {
  DCHECK(task_impl_);
  return task_impl_;
}

bool MuleTask::Resume() {
  bool ret = TaskBase::Resume();
  SetLocalLandFileName();
  return ret;
}

}  // namespace nas
