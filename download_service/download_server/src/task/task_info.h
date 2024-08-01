/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-20 16:04:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_TASK_INFO_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_TASK_INFO_H_

#include <base/files/file_path.h>
#include <base/synchronization/lock.h>
#include <memory>
#include "task_common_define.h"

namespace nas {
using FileProgressListPtr = std::shared_ptr<std::map<std::string, double>>;
// 任务进度类
class TaskProgressInfo {
 public:
  TaskProgressInfo();
  ~TaskProgressInfo() = default;
  TaskProgressInfo(const TaskProgressInfo& other);

  void SetTaskName(const std::string& task_name);
  std::string GetTaskName();

  void SetTotalProgress(double progress);
  double GetTotalProgress();

  void SetDownloadSpeed(double speed);
  double GetDownloadSpeed();

  void SetUploadSpeed(double speed);
  double GetUploadSpeed();

  void SetFileListProgress(const std::string& file_name, double progress);
  FileProgressListPtr GetFileListProgress();

  void SetErrorCode(DownloadErrorCode err_code);
  DownloadErrorCode GetErrorCode();

  void Reset();

 private:
  std::string task_name_;                     // 任务名称
  double total_progress_ = 0.0;  // 下载总进度，0-100
  double download_speed_ = 0.0;  // 下载速度，单位字节/s
  double upload_speed_ = 0.0;    // 上传速度，单位字节/s
  FileProgressListPtr file_list_progress_ = nullptr;    // 文件列表进度
  DownloadErrorCode err_code_ = DownloadErrorCode::kNormal;  // 错误码
};
using TaskProgressInfoPtr = std::shared_ptr<TaskProgressInfo>;

class TaskBaseInfo {
 public:
  TaskBaseInfo();
  ~TaskBaseInfo() = default;
  TaskBaseInfo(const TaskBaseInfo& other) = default;

  void SetTaskName(const std::string& name);
  void SetTaskFileSize(int64_t file_size);
  void SetTaskFileType(const std::string& file_type);
  void SetTaskLandFileName(const std::string& name);
  //void SetTaskAttribute(const FileAttribute& attribute);
  FileAttribute GetTaskAttribute();

  void SetFileList(const std::string& file_list);
  const std::string& GetFileList();

 private:
  FileAttribute task_attribute_;
  std::string file_list_;
};
using TaskBaseInfoPtr = std::shared_ptr<TaskBaseInfo>;

class TaskInfo {
 public:
  TaskInfo();
  ~TaskInfo() = default;
  TaskInfo(const TaskInfo& other);

  void SetTaskParam(TaskParamPtr param);
  const TaskParamPtr GetTaskParam();

  void SetTaskBaseInfo(TaskBaseInfoPtr base_info);
  TaskBaseInfoPtr GetTaskBaseInfo();

  void SetProgressInfo(TaskProgressInfoPtr progress_info);
  TaskProgressInfoPtr GetProgressInfo();

  void SetTaskStatus(TaskStatus status);
  TaskStatus GetTaskStatus();

  void SetTaskCategory(TaskStatus status);
  void SetTaskCategory(const TaskCategory& category);
  TaskCategory GetTaskCategory();

  void SetTaskTimeStamp(TaskTimeStampPtr time_stamp);
  TaskTimeStampPtr GetTaskTimeStamp();

  void SetIsValid(bool is_valid);
  bool IsValid();

  void SetIsStarted(bool is_started);
  bool IsStarted();

  void SetIsRetry(bool is_retry);
  bool IsRetry();

 private:
  TaskParamPtr task_param_ = nullptr;
  TaskBaseInfoPtr task_base_info_ = nullptr;
  TaskProgressInfoPtr task_progress_info_ = nullptr;
  TaskStatus task_status_ = TaskStatus::kWaitting;
  TaskCategory task_category_ = TaskCategory::kDownloadingTask;
  TaskTimeStampPtr task_time_stamp_ = nullptr;
  bool is_valid_ = true;
  bool is_started_ = false;
  bool is_retry_ = false;
};
using TaskInfoPtr = std::shared_ptr<TaskInfo>;
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_TASK_INFO_H_