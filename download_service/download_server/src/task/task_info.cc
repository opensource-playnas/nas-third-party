/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-20 16:04:06
 */

#include "task_info.h"

namespace nas {
TaskProgressInfo::TaskProgressInfo(const TaskProgressInfo& other) {
  task_name_ = other.task_name_;
  total_progress_ = other.total_progress_;
  download_speed_ = other.download_speed_;
  upload_speed_ = other.upload_speed_;
  err_code_ = other.err_code_;

  file_list_progress_ = std::make_shared<std::map<std::string, double>>(
      *other.file_list_progress_);
}

TaskProgressInfo::TaskProgressInfo() {
  file_list_progress_ = std::make_shared<std::map<std::string, double>>();
}

void TaskProgressInfo::SetTaskName(const std::string& task_name) {
  task_name_ = task_name;
}

std::string TaskProgressInfo::GetTaskName() {
  return task_name_;
}

void TaskProgressInfo::SetTotalProgress(double progress) {
  total_progress_ = progress;
}

double TaskProgressInfo::GetTotalProgress() {
  return total_progress_;
}

void TaskProgressInfo::SetDownloadSpeed(double speed) {
  download_speed_ = speed;
}

double TaskProgressInfo::GetDownloadSpeed() {
  return download_speed_;
}

void TaskProgressInfo::SetUploadSpeed(double speed) {
  upload_speed_ = speed;
}

double TaskProgressInfo::GetUploadSpeed() {
  return upload_speed_;
}

void TaskProgressInfo::SetFileListProgress(const std::string& file_name,
                                           double progress) {
  (*file_list_progress_)[file_name] = progress;
}

FileProgressListPtr TaskProgressInfo::GetFileListProgress() {
  return file_list_progress_;
}

void TaskProgressInfo::SetErrorCode(DownloadErrorCode err_code) {
  err_code_ = err_code;
}

DownloadErrorCode TaskProgressInfo::GetErrorCode() {
  return err_code_;
}

void TaskProgressInfo::Reset() {
  err_code_ = DownloadErrorCode::kNormal;
  download_speed_ = 0;
  upload_speed_ = 0;
  total_progress_ = 0;
}

TaskBaseInfo::TaskBaseInfo() {}

void TaskBaseInfo::SetTaskName(const std::string& name) {
  task_attribute_.file_name = name;
}

void TaskBaseInfo::SetTaskFileSize(int64_t file_size) {
  task_attribute_.file_size = file_size;
}

void TaskBaseInfo::SetTaskFileType(const std::string& file_type) {
  task_attribute_.file_type = file_type;
}

void TaskBaseInfo::SetTaskLandFileName(const std::string& name) {
  task_attribute_.local_land_file_name = name;
}

// void TaskBaseInfo::SetTaskAttribute(const FileAttribute& attribute) {
//   task_attribute_ = attribute;
// }

FileAttribute TaskBaseInfo::GetTaskAttribute() {
  return task_attribute_;
}

void TaskBaseInfo::SetFileList(const std::string& file_list) {
  file_list_ = file_list;
}

const std::string& TaskBaseInfo::GetFileList() {
  return file_list_;
}

TaskInfo::TaskInfo() {
  task_base_info_ = std::make_shared<TaskBaseInfo>();
  task_progress_info_ = std::make_shared<TaskProgressInfo>();
  task_time_stamp_ = std::make_shared<TaskTimeStamp>();
}

TaskInfo::TaskInfo(const TaskInfo& other) {
  task_param_ = std::make_shared<TaskParam>(*other.task_param_);
  task_base_info_ = std::make_shared<TaskBaseInfo>(*other.task_base_info_);
  task_progress_info_ =
      std::make_shared<TaskProgressInfo>(*other.task_progress_info_);
  task_time_stamp_ = std::make_shared<TaskTimeStamp>(*other.task_time_stamp_);

  task_status_ = other.task_status_;
  task_category_ = other.task_category_;
  is_valid_ = other.is_valid_;
  is_started_ = other.is_started_;
  is_retry_ = other.is_retry_;
}

void TaskInfo::SetTaskParam(TaskParamPtr param) {
  task_param_ = param;
}

const TaskParamPtr TaskInfo::GetTaskParam() {
  return task_param_;
}

void TaskInfo::SetTaskBaseInfo(TaskBaseInfoPtr base_info) {
  task_base_info_ = base_info;
}

TaskBaseInfoPtr TaskInfo::GetTaskBaseInfo() {
  return task_base_info_;
}

void TaskInfo::SetProgressInfo(TaskProgressInfoPtr progress_info) {
  task_progress_info_ = progress_info;
}

TaskProgressInfoPtr TaskInfo::GetProgressInfo() {
  return task_progress_info_;
}

void TaskInfo::SetTaskStatus(TaskStatus status) {
  task_status_ = status;
  SetTaskCategory(status);
}

TaskStatus TaskInfo::GetTaskStatus() {
  return task_status_;
}

void TaskInfo::SetTaskCategory(TaskStatus status) {
  switch (status) {
    case TaskStatus::kFinished:
      task_category_ = TaskCategory::kFinishedTask;
      break;
    case TaskStatus::kDeleted:
      task_category_ = TaskCategory::kRecyclebinTask;
      break;
    case TaskStatus::kCompletedDeleted:
      task_category_ = TaskCategory::kUnkownnTask;
      break;
    case TaskStatus::kSeeding:
      task_category_ = TaskCategory::kSeedingTask;
      break;
    case TaskStatus::kFailed:
      // 失败任务状态，其分类可能是下载中，也可能是其它分类，保留原有的分类不变
      break;
    default:
      task_category_ = TaskCategory::kDownloadingTask;
      break;
  }
}

void TaskInfo::SetTaskCategory(const TaskCategory& category) {
  task_category_ = category;
}

TaskCategory TaskInfo::GetTaskCategory() {
  return task_category_;
}

void TaskInfo::SetTaskTimeStamp(TaskTimeStampPtr time_stamp) {
  task_time_stamp_ = time_stamp;
}

TaskTimeStampPtr TaskInfo::GetTaskTimeStamp() {
  return task_time_stamp_;
}

void TaskInfo::SetIsValid(bool is_valid) {
  is_valid_ = is_valid;
}

bool TaskInfo::IsValid() {
  return is_valid_;
}

void TaskInfo::SetIsStarted(bool is_started) {
  is_started_ = is_started;
}

bool TaskInfo::IsStarted() {
  return is_started_;
}

void TaskInfo::SetIsRetry(bool is_retry) {
  is_retry_ = is_retry;
}

bool TaskInfo::IsRetry() {
  return is_retry_;
}

}  // namespace nas
