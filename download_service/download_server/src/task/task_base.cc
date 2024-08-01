/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:46
 */
#include "task_base.h"
#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_task_runner_handle.h"

#include "nas/common/message_center_define.hpp"
#include "nas/common/path/file_path_unit.h"
#include "nas/download_service/download_server/src/task_ref_count.h"
#include "nas/download_service/download_server/src/utils/utils.h"

namespace nas {
static const int kDownloadTemplateId = 201;  // 任务完成发送消息中心的模板id

TaskBase::TaskBase(TaskParamPtr param, const ContextField& context_field)
    : context_field_(context_field) {
  if (!param->is_parse_task) {
     // 当前线程不能是线程池，Link解析任务不是单线程
      task_runner_ = base::ThreadTaskRunnerHandle::Get();
  }

  if (!IsResumeFromDb(param)) {
    task_info_ = std::make_shared<TaskInfo>();
    task_info_->SetTaskParam(param);
    task_info_->SetTaskStatus(TaskStatus::kWaitting);
    SetCreateTimeStamp();
  }
}

TaskBase::~TaskBase() {}

bool TaskBase::Pause() {
  if (GetDownloadTaskImp()->Pause()) {
    SetTaskStatus(TaskStatus::kPaused);
    task_info_->GetProgressInfo()->SetDownloadSpeed(0);
    return true;
  }

  return false;
}

bool TaskBase::Resume() {
  TaskStatus status = GetTaskStatus();
  bool is_failed = (status == TaskStatus::kFailed) || (!task_info_->IsValid());
  if (GetDownloadTaskImp()->Resume(is_failed)) {
    SetTaskStatus(TaskStatus::kDownloading);
    task_info_->SetIsValid(true);
    return true;
  }

  return false;
}

bool TaskBase::Retry() {
  if (!GetDownloadTaskImp()) {
    return false;
  }

  SetCreateTimeStamp();
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  progress_info->Reset();
  task_info_->SetIsRetry(true);

  return true;
}

bool TaskBase::Cancel(bool is_delete_local_file) {
  nas::ReleaseCallback callback = std::bind(
      &TaskBase::ExceCancel, this, is_delete_local_file, std::placeholders::_1);
  return TaskRefCount::GetInstance()->Release(GetTaskHash(), callback);
}

bool TaskBase::DeleteToRecycleBin() {
  if (!GetDownloadTaskImp()) {
    return false;
  }

  if (GetTaskStatus() == TaskStatus::kFailed || GetDownloadTaskImp()->Pause()) {
    SetTaskStatus(TaskStatus::kDeleted);
    TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();
    task_time_stamp->delete_time = nas::utils::GetCurrnetTime();
    DownloadStoreHelper::GetInstance()->UpdateTaskInfo(context_field_.user_id(),
                                                       task_info_);
  }

  return true;
}

bool TaskBase::Restore() {
  if (!GetDownloadTaskImp()) {
    return false;
  }

  switch (pre_status_) {
    case TaskStatus::kFinished:
      SetTaskStatus(TaskStatus::kFinished);
      break;
    case TaskStatus::kParsing:
      SetTaskStatus(TaskStatus::kParsing);
      SetCreateTimeStamp();
      break;
    case TaskStatus::kPaused:
      SetTaskStatus(TaskStatus::kPaused);
      SetCreateTimeStamp();
      break;
    case TaskStatus::kSeeding:
      Resume();
      SetTaskStatus(TaskStatus::kSeeding);
      break;
    case TaskStatus::kFailed:
      SetTaskStatus(TaskStatus::kFailed);
      task_info_->SetTaskCategory(TaskCategory::kDownloadingTask);
      Resume();
      SetCreateTimeStamp();
      break;
    case TaskStatus::kWaitting:
      SetTaskStatus(TaskStatus::kWaitting);
      SetCreateTimeStamp();
      break;
    case TaskStatus::kDownloading:
      SetTaskStatus(TaskStatus::kWaitting);
      SetCreateTimeStamp();
      break;
    default: {
      if (IsDownloadFinished()) {
        SetTaskStatus(TaskStatus::kFinished);
        break;
      }
      SetTaskStatus(TaskStatus::kPaused);
    }
  }

  return true;
}

TaskParamPtr TaskBase::GetTaskParam() {
  if (task_info_) {
    return task_info_->GetTaskParam();
  }
  return nullptr;
}

void TaskBase::SetTaskStatus(const TaskStatus& status) {
  if (!task_info_) {
    return;
  }
  pre_status_ = task_info_->GetTaskStatus();
  task_info_->SetTaskStatus(status);
  DownloadStoreHelper::GetInstance()->UpdateTaskInfo(context_field_.user_id(),
                                                     task_info_);
}

TaskStatus TaskBase::GetTaskStatus() {
  if (!task_info_) {
    return TaskStatus::kFailed;
  }
  return task_info_->GetTaskStatus();
}

void TaskBase::SetContextField(const ContextField& context_field) {
  context_field_ = context_field;
}

ContextField TaskBase::GetContextField() {
  return context_field_;
}

nas::TaskCategory TaskBase::GetTaskCategory() {
  if (!task_info_) {
    return nas::TaskCategory::kDownloadingTask;
  }
  return task_info_->GetTaskCategory();
}

TaskInfoPtr TaskBase::GetTaskInfo() {
  return task_info_;
}

bool TaskBase::IsDownloadFinished() {
  if (100 - task_info_->GetProgressInfo()->GetTotalProgress() < nas::kDValue) {
    return true;
  }
  return false;
}

bool TaskBase::DeleteLocalFile() {
  bool ret = true;
  do {
    TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
    FileAttribute file_attribute = task_base_info->GetTaskAttribute();

    if (file_attribute.file_name.empty()) {
      LOG(INFO) << "file name is empty";
      break;
    }

    base::FilePath path =
        path_unit::BaseFilePathFromU8(task_info_->GetTaskParam()->save_path);
    path = path.Append(path_unit::BaseFilePathFromU8(file_attribute.file_name));
    if (base::PathExists(path)) {
      ret = base::DeletePathRecursively(path);
    }
    LOG(INFO) << "delete local file " << (ret ? "success" : "failed")
              << ", path = " << path;

  } while (0);
  return ret;
}

void TaskBase::PublishTaskBaseInfo() {
  const TaskParamPtr task_param = task_info_->GetTaskParam();
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();
  FileAttribute file_attribute = task_base_info->GetTaskAttribute();

  base::Value::Dict message_data;
  message_data.Set("task_id", task_param->id);
  message_data.Set("save_path", task_param->save_path);
  message_data.Set("source", task_param->url);
  message_data.Set("converted_source", converted_source_);
  std::string file_list = task_base_info->GetFileList();
  message_data.Set("file_list_info", file_list);
  message_data.Set("name", file_attribute.file_name);
  message_data.Set("local_land_name", file_attribute.local_land_file_name);
  message_data.Set("total_size", std::to_string(file_attribute.file_size));
  message_data.Set("task_status", GetTaskStatus());
  message_data.Set("task_category", GetTaskCategory());
  message_data.Set("create_timestamp",
                   std::to_string(task_time_stamp->create_time));
  message_data.Set("file_type", file_attribute.file_type);
  message_data.Set("download_type", task_param->type);
  message_data.Set("task_is_valid", task_info_->IsValid());

  nas::utils::Notify(kTaskBaseInfoCallId, std::move(message_data),
                     context_field_);
  LOG(INFO) << "task_id = " << task_param->id
            << ", name = " << file_attribute.file_name
            << ", publish base info!";
}

base::Value::Dict TaskBase::BuildDownloadingData() {
  const TaskParamPtr task_param = task_info_->GetTaskParam();
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  FileAttribute file_attribute = task_base_info->GetTaskAttribute();
  if (GetTaskStatus() == TaskStatus::kPaused) {
    progress_info->SetDownloadSpeed(0);
    progress_info->SetUploadSpeed(0);
  }

  base::Value::Dict message_data;
  message_data.Set("task_id", task_param->id);

  message_data.Set("converted_source", converted_source_);

  std::string file_list = task_base_info->GetFileList();
  message_data.Set("file_list_info", std::move(file_list));
  message_data.Set("save_path", task_param->save_path);
  message_data.Set("name", file_attribute.file_name);
  message_data.Set("local_land_name", file_attribute.local_land_file_name);
  message_data.Set("total_size", std::to_string(file_attribute.file_size));
  message_data.Set("total_progress", progress_info->GetTotalProgress());
  message_data.Set("download_speed", progress_info->GetDownloadSpeed());
  message_data.Set("upload_speed", progress_info->GetUploadSpeed());

  double completed_size =
      file_attribute.file_size * progress_info->GetTotalProgress() / 100.0;
  message_data.Set("completed_size", std::to_string(completed_size));

  double time_remaining = -1;  // 秒
  if (progress_info->GetDownloadSpeed() != 0) {
    time_remaining = (file_attribute.file_size - completed_size) /
                     progress_info->GetDownloadSpeed();
  }

  message_data.Set("time_remaining", time_remaining);
  message_data.Set("task_status", GetTaskStatus());
  message_data.Set("task_category", GetTaskCategory());
  message_data.Set("download_type", task_param->type);
  message_data.Set("err_code", utils::DownloadErrNoToOutErrNo(
                                   progress_info->GetErrorCode()));
  nas::CallResult call_result =
      utils::ErrNoToCallResult(progress_info->GetErrorCode());
  message_data.Set("msg", call_result.msg);

  DownloadStoreHelper::GetInstance()->UpdateTaskInfo(context_field_.user_id(),
                                                     task_info_);

  return message_data;
}

void TaskBase::PublishInvalidTaskInfo() {
  const TaskParamPtr task_param = task_info_->GetTaskParam();
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  base::Value::Dict message_data;
  message_data.Set("task_id", task_param->id);
  message_data.Set("task_status", GetTaskStatus());
  message_data.Set("task_category", GetTaskCategory());
  message_data.Set("task_is_valid", false);
  message_data.Set("err_code", utils::DownloadErrNoToOutErrNo(
                                   progress_info->GetErrorCode()));
  nas::CallResult call_result =
      utils::ErrNoToCallResult(progress_info->GetErrorCode());
  message_data.Set("msg", call_result.msg);
  nas::utils::Notify(kInvalidTaskInfoCallId, std::move(message_data),
                     context_field_);
}

void TaskBase::PublishFailedTaskInfo() {
  const TaskParamPtr task_param = task_info_->GetTaskParam();
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  FileAttribute file_attribute = task_base_info->GetTaskAttribute();
  TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();

  base::Value::Dict message_data;
  message_data.Set("task_id", task_param->id);
  message_data.Set("name", file_attribute.file_name);
  message_data.Set("task_status", GetTaskStatus());
  message_data.Set("task_category", GetTaskCategory());
  message_data.Set("create_timestamp",
                   std::to_string(task_time_stamp->create_time));
  nas::CallResult call_result =
      utils::ErrNoToCallResult(progress_info->GetErrorCode());
  message_data.Set("err_code", utils::DownloadErrNoToOutErrNo(
                                   progress_info->GetErrorCode()));
  message_data.Set("msg", call_result.msg);
  nas::utils::Notify(kFailedTaskInfoCallId, std::move(message_data),
                     context_field_);
  LOG(INFO) << "task_id = " << task_param->id
            << ", name = " << file_attribute.file_name
            << ", publish failed info!";

  SendTaskEndMessage(false, file_attribute.file_name);
}

void TaskBase::PublishFinishedTaskInfo() {
  const TaskParamPtr task_param = task_info_->GetTaskParam();
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();
  FileAttribute file_attribute = task_base_info->GetTaskAttribute();
  task_base_info->SetTaskLandFileName(file_attribute.file_name);

  base::Value::Dict message_data;
  message_data.Set("task_id", task_param->id);
  message_data.Set("save_path", task_param->save_path);
  message_data.Set("name", file_attribute.file_name);
  message_data.Set("local_land_name", file_attribute.file_name);
  message_data.Set("task_status", GetTaskStatus());
  message_data.Set("task_category", GetTaskCategory());
  message_data.Set("task_is_valid", task_info_->IsValid());
  if (file_attribute.file_size == 0) {
    base::FilePath path = nas::path_unit::BaseFilePathFromU8(
        task_param->save_path + '/' + file_attribute.file_name);
    int64_t file_size = 0;
    base::GetFileSize(path, &file_size);
    task_base_info->SetTaskFileSize(file_size);
  }
  message_data.Set("total_size", std::to_string(file_attribute.file_size));
  task_time_stamp->finish_time = nas::utils::GetCurrnetTime();
  message_data.Set("finish_timestamp",
                   std::to_string(task_time_stamp->finish_time));
  message_data.Set("download_type", task_param->type);
  message_data.Set("err_code", 0);
  message_data.Set("msg", "");
  nas::utils::Notify(kFinishedTaskInfoCallId, std::move(message_data),
                     context_field_);
  LOG(INFO) << "task_id = " << task_param->id
            << ", name = " << file_attribute.file_name << ", finished!";
}

void TaskBase::SendTaskEndMessage(bool success, const std::string& task_name) {
  std::string title = "远程下载任务完成";
  std::string sub_title;
  std::string action = common::message_action::kDownloadFile;
  std::string action_status = common::message_status::kSuccess;
  if (success) {
    sub_title =
        base::StringPrintf("您的远程下载任务已完成: %s", task_name.c_str());
  } else {
    sub_title =
        base::StringPrintf("您的远程下载任务失败: %s", task_name.c_str());
    action_status = common::message_status::kFailed;
  }

}

base::Value::Dict TaskBase::BuildMessageData(const TaskOperation& operation) {
  const TaskParamPtr task_param = task_info_->GetTaskParam();
  base::Value::Dict dict;
  dict.Set("task_id", task_param->id);
  dict.Set("task_status", GetTaskStatus());
  dict.Set("task_category", GetTaskCategory());
  dict.Set("task_is_valid", task_info_->IsValid());
  TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();
  dict.Set("create_timestamp", std::to_string(task_time_stamp->create_time));

  switch (operation) {
    case TaskOperation::kPause:
      break;
    case TaskOperation::kPauseAll:
      break;
    case TaskOperation::kResume:
      break;
    case TaskOperation::kResumeAll:
      break;
    case TaskOperation::kDelete:
    case TaskOperation::kDeleteCategory: {
      TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();
      dict.Set("delete_timestamp",
               std::to_string(task_time_stamp->delete_time));
    } break;
    case TaskOperation::kRetry:
    case TaskOperation::kRetryAll: {
      FileAttribute file_attribute =
          task_info_->GetTaskBaseInfo()->GetTaskAttribute();
      TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
      dict.Set("total_progress", progress_info->GetTotalProgress());
      dict.Set("download_speed", progress_info->GetDownloadSpeed());
      double completed_size =
          file_attribute.file_size * progress_info->GetTotalProgress() / 100.0;
      dict.Set("completed_size", std::to_string(completed_size));
    } break;
    case TaskOperation::kRestore:
    case TaskOperation::kRestoreAll:
      break;
    default:
      break;
  }

  return dict;
}

bool TaskBase::CheckTaskIsFailed() {
  TaskProgressInfoPtr progress_info = task_info_->GetProgressInfo();
  TaskStatus status = GetTaskStatus();
  if (status != TaskStatus::kFailed && status != TaskStatus::kDeleted &&
      status != TaskStatus::kFinished &&
      progress_info->GetErrorCode() != DownloadErrorCode::kNormal) {
    UpdateAndPublishFailedInfo();
    return false;
  } else if (status == TaskStatus::kFailed) {
    return false;
  }
  return true;
}

void TaskBase::UpdateAndPublishFailedInfo() {
  SetTaskStatus(TaskStatus::kFailed);
  PublishFailedTaskInfo();
  DownloadStoreHelper::GetInstance()->UpdateTaskInfo(context_field_.user_id(),
                                                     task_info_);
}

bool TaskBase::Valid() {
  bool is_finished = IsDownloadFinished() ? true : false;
  if (!GetDownloadTaskImp()->Valid(is_finished)) {
    return false;
  }
  return true;
}

base::Value::Dict TaskBase::GetFileList() {
  base::Value::Dict file_list_dict;
  TaskBaseInfoPtr task_base_info = task_info_->GetTaskBaseInfo();
  std::string file_list = task_base_info->GetFileList();
  absl::optional<base::Value> json_value = base::JSONReader::Read(file_list);
  if (!json_value) {
    return file_list_dict;
  }

  if (base::Value::Dict* dict = json_value->GetIfDict()) {
    file_list_dict = std::move(*dict);
  }
  return file_list_dict;
}

void TaskBase::SetCreateTimeStamp() {
  TaskTimeStampPtr task_time_stamp = task_info_->GetTaskTimeStamp();
  task_time_stamp->create_time = nas::utils::GetCurrnetTime();
}

bool TaskBase::IsParseFinish() {
  return parse_finished_ ||
         !task_info_->GetTaskBaseInfo()->GetFileList().empty();
}

bool TaskBase::IsResumeFromDb(const TaskParamPtr param) {
  return param->is_from_db_resume ? true : false;
}

bool TaskBase::ExceCancel(bool is_delete_local_file, bool real_delete) {
  bool ret = true;
  do {
    if (!task_info_->IsStarted()) {
      SetTaskStatus(TaskStatus::kCompletedDeleted);
      break;
    }

    if (real_delete) {
      if (GetDownloadTaskImp()->Cancel(is_delete_local_file)) {
        SetTaskStatus(TaskStatus::kCompletedDeleted);
      } else {
        ret = false;
      }
    }
  } while (0);

  return ret;
}

std::string TaskBase::GetTaskHash() {
  std::string hash; 
  const char* buf = GetDownloadTaskImp()->GetTaskHash();
  if (buf && task_info_) {
    hash = buf;
  }
  if (buf) {
    GetDownloadTaskImp()->ReleaseBuf(buf);
  }
  return hash;
}

}  // namespace nas
