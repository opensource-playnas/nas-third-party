/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:55:38
 */

#include "link_task.h"
#include "base/strings/stringprintf.h"
#include "nas/common/path/file_path_unit.h"

namespace nas {

LinkTask::LinkTask(TaskParamPtr param, const ContextField& context_field)
    : TaskBase(param, context_field) {
    }

LinkTask::~LinkTask() {}

bool LinkTask::Parse() {
  bool ret = false;
  do {
    TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
    TaskParamPtr task_param = task_info_->GetTaskParam();

    ParseResult parse_result;
    if (task_param->file_size == 0) {
      int err = SyncParse(&parse_result);
      task_base_info->SetTaskName(parse_result.file_info.file_name);
      task_base_info->SetTaskFileSize(parse_result.file_info.file_size);
      task_base_info->SetTaskFileType(std::to_string(
          nas::path_unit::GetFileType(parse_result.file_info.file_name)));
      if (err) {
        TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
        progress_info->SetErrorCode(DownloadErrorCode(err));
        SetTaskStatus(TaskStatus::kFailed);
        PublishFailedTaskInfo();
        LOG(ERROR) << "parse curl link failed!";
        break;
      }
    } else {
      task_base_info->SetTaskName(task_param->file_name);
      task_base_info->SetTaskFileSize(task_param->file_size);
      task_base_info->SetTaskFileType(
          std::to_string(nas::path_unit::GetFileType(task_param->file_name)));
    }

    if (!task_info_->IsRetry()) {
      PublishTaskBaseInfo();
    }
    ret = true;
  } while (0);

  SetLocalLandFileName();
  return ret;
}

DownloadErrorCode LinkTask::Start() {
  if (task_info_ && task_info_->IsStarted()) {
    return DownloadErrorCode::kNormal;
  }

  SetTaskStatus(TaskStatus::kDownloading);
  DownloadErrorCode ret = GetDownloadTaskImp()->Start();
  if (ret == DownloadErrorCode::kNormal) {
    task_info_->SetIsStarted(true);
    task_info_->SetIsValid(true);
  }
  return ret;
}

void LinkTask::ResumeDataFromDb(TaskInfoPtr task_info) {
  task_info_ = task_info;
  if (task_info_ && task_info_->GetTaskStatus() == TaskStatus::kDownloading) {
    ResumeFromDb();
  }
  SetLocalLandFileName();
}

void LinkTask::SetLocalLandFileName() {
  // 1、链接任务临时文件名格式为：文件名.task_id.nas.tmp
  //    e.g. file_name 为
  //    WeChatSetup.exe，则落地的临时文件为WeChatSetup.exe.task_id.nas.tmp
  // 2、任务下载完成后本地落地文件名为task_name
  if (!task_info_) {
    return;
  }
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  std::string task_name = task_base_info->GetTaskAttribute().file_name;
  TaskStatus status = task_info_->GetTaskStatus();
  if (status == TaskStatus::kFinished) {
    task_base_info->SetTaskLandFileName(task_name);
  } else {
    TaskParamPtr task_param = task_info_->GetTaskParam();
    std::string local_land_file_name = base::StringPrintf(
        "%s.%s.nas.tmp", task_name.c_str(), task_param->id.c_str());
    task_base_info->SetTaskLandFileName(local_land_file_name);
  }
}

void LinkTask::UpdateProgressInfo(base::Value::Dict* result) {
  DownloadProgressInfo download_progress_info;
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  download_progress_info.total_progress = progress_info->GetTotalProgress();
  GetDownloadTaskImp()->GetProgressInfo(&download_progress_info);

  std::string task_name = download_progress_info.file_name;
  if (!task_name.empty()) {
    task_info_->GetTaskBaseInfo()->SetTaskName(task_name);
    progress_info->SetTaskName(task_name);
  }
  progress_info->SetTaskName(download_progress_info.file_name);
  progress_info->SetTotalProgress(download_progress_info.total_progress);
  progress_info->SetDownloadSpeed(download_progress_info.download_speed);
  progress_info->SetUploadSpeed(download_progress_info.upload_speed);
  progress_info->SetErrorCode(download_progress_info.err_code);

  if (IsDownloadFinished()) {
    SetTaskStatus(TaskStatus::kFinished);
    PublishFinishedTaskInfo();
    SendTaskEndMessage(
        true, task_info_->GetTaskBaseInfo()->GetTaskAttribute().file_name);
  }

  DownloadStoreHelper::GetInstance()->UpdateTaskInfo(context_field_.user_id(),
                                                     task_info_);
  *result = BuildDownloadingData();
}

void LinkTask::AssembleDownloadParam(DownloadParam* param,
                                     const TaskParamPtr task_param) {
  if (sizeof(param->id) >= task_param->id.size()) {
    memcpy(param->id, task_param->id.c_str(), task_param->id.size());
  }

  if (sizeof(param->url) >= task_param->url.size()) {
    memcpy(param->url, task_param->url.c_str(), task_param->url.size());
  }

  if (sizeof(param->save_path) >= task_param->save_path.size()) {
    memcpy(param->save_path, task_param->save_path.c_str(),
           task_param->save_path.size());
  }

  if (sizeof(param->file_name) >= task_param->file_name.size()) {
    memcpy(param->file_name, task_param->file_name.c_str(),
           task_param->file_name.size());
  }
}

DownloadErrorCode LinkTask::CheckCondition() {
  DownloadErrorCode err_code = DownloadErrorCode::kNormal;
  if (!nas::utils::IsValid(GetTaskParam()->url)) {
    LOG(ERROR) << "url is not valid, url = " << GetTaskParam()->url;
    err_code = DownloadErrorCode::kCurlUrlInvalid;
  }
  return err_code;
}

}  // namespace nas
