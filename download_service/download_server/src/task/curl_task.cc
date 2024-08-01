/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:55:38
 */

#include "curl_task.h"

namespace nas {

CurlTask::CurlTask(TaskParamPtr param, const ContextField& context_field)
    : LinkTask(param, context_field) {
  if (!task_impl_) {
    task_impl_ =
        DownloadInterfaceHelper::GetInstance()->CreateCurlDownloadTask();
  }
  SetDownloadParam(param);
  param->hash = GetTaskHash();
}

CurlTask::~CurlTask() {
  if (task_impl_) {
    LOG(WARNING) << "release curl task id = " << task_info_->GetTaskParam()->id
                 << ", name = " << task_info_->GetTaskParam()->file_name;
    DownloadInterfaceHelper::GetInstance()->ReleaseCurlTask(task_impl_);
    task_impl_ = NULL;
  }
}

bool CurlTask::Resume() {
  bool ret = TaskBase::Resume();
  if (task_runner_) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&CurlTask::ReStart, base::Unretained(this)));
  }
  SetLocalLandFileName();
  return ret;
}

bool CurlTask::Retry() {
  if (task_impl_) {
    task_impl_->Reset();
  }
  return TaskBase::Retry();
}

int CurlTask::SyncParse(ParseResult* result) {
  TaskParamPtr task_param = task_info_->GetTaskParam();
  return task_impl_->ParseLink(task_param->url.c_str(), result);
}

bool CurlTask::ResumeFromDb() {
  if (!task_impl_) {
    return false;
  }
  bool ret = task_impl_->ResumeBreakPoint();
  if (ret) {
    task_info_->SetIsStarted(true);
  } else {
    TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
    progress_info->SetErrorCode(DownloadErrorCode::kFileMissing);
  }

  return ret;
}

void CurlTask::SetDownloadParam(const TaskParamPtr param) {
  DownloadParam download_param;
  AssembleDownloadParam(&download_param, param);
  DCHECK(task_impl_);
  task_impl_->SetParam(download_param);
  int64_t curl_max_download_speed =
      SystemCloudConfig::GetInstance()->GetCurlMaxDownloadSpeed();
  task_impl_->SetMaxDownloadSpeed(curl_max_download_speed);
}

DownloadTaskInterface* CurlTask::GetDownloadTaskImp() {
  DCHECK(task_impl_);
  return task_impl_;
}

void CurlTask::ReStart() {
  task_impl_->TryReStart();
}

}  // namespace nas
