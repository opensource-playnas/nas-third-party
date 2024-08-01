/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-06 13:26:42
 */
#include "user_self_mgr.h"

#include "base/base64.h"
#include "base/guid.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/escape.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

#include "nas/common//path/file_path_unit.h"
#include "nas/common/nas_config.h"
#include "nas/common/path/path_protected.h"

#include "nas/download_service/download_server/src/db/download_store_helper.h"
#include "nas/download_service/download_server/src/download_error_desc.h"
#include "nas/download_service/download_server/src/task/bt_magnet_task.h"
#include "nas/download_service/download_server/src/task/bt_task_base.h"
#include "nas/download_service/download_server/src/task/bt_torrent_task.h"
#include "nas/download_service/download_server/src/task/curl_task.h"
#include "nas/download_service/download_server/src/task/mule_task.h"
#include "nas/download_service/download_server/src/task_factory.h"
#include "nas/download_service/download_server/src/task_ref_count.h"
#include "nas/download_service/download_server/src/utils/utils.h"

namespace nas {
bool AscendingByCreateTime(const TaskBasePtr& left, const TaskBasePtr& right) {
  return left->GetTaskInfo()->GetTaskTimeStamp()->create_time <
         right->GetTaskInfo()->GetTaskTimeStamp()->create_time;
}

bool DecendingByCreateTime(const TaskBasePtr& left, const TaskBasePtr& right) {
  return left->GetTaskInfo()->GetTaskTimeStamp()->create_time >
         right->GetTaskInfo()->GetTaskTimeStamp()->create_time;
}

bool AscendingByTaskSise(const TaskBasePtr& left, const TaskBasePtr& right) {
  int64_t left_size =
      left->GetTaskInfo()->GetTaskBaseInfo()->GetTaskAttribute().file_size;
  int64_t right_size =
      right->GetTaskInfo()->GetTaskBaseInfo()->GetTaskAttribute().file_size;

  if (left_size != right_size) {
    return left_size < right_size;
  }
  return left->GetTaskInfo()->GetTaskTimeStamp()->create_time <
         right->GetTaskInfo()->GetTaskTimeStamp()->create_time;
}

UserSelfMgr::UserSelfMgr(
    const ContextField& context_field,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : user_id_(context_field.user_id()), task_runner_(task_runner) {
  DCHECK(task_runner_);
  // 从数据库恢复时nas_token为空
  if (!context_field.nas_token().empty()) {
    user_info_ = std::make_shared<UserInfo>(user_id_);
    DownloadStoreHelper::GetInstance()->AddUserInfo(user_id_, user_info_);
  }
}

UserSelfMgr::~UserSelfMgr() {
  LOG(INFO) << "~UserSelfMgr";
}

void UserSelfMgr::Stop() {
  StopTimer();
}

void UserSelfMgr::Reset(base::WaitableEvent& wait_event) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&UserSelfMgr::Reset, shared_from_this(),
                                  std::ref(wait_event)));
    return;
  }

  for (auto& [task_id, task] : task_list_) {
    if (task) {
      LOG(WARNING) << "reset task " << task_id;
      task->Pause();
      task->Cancel(false);
    }
  }
  task_list_.clear();
  LOG(WARNING) << "reset task: clear task list";
  wait_event.Signal();
}

void UserSelfMgr::CreateLinkTask(
    const download::v1::CreateLinkDownloadTaskRequest* request,
    const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::CreateLinkDownloadTaskRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::CreateLinkTask, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  std::vector<TaskBasePtr> need_parse_tasks;

  int size = request->source_list_size();
  for (int i = 0; i < size; ++i) {
    download_data_struct::SourceData source_data = request->source_list(i);

    TaskParamPtr task_param = std::make_shared<TaskParam>();
    task_param->id = source_data.task_id();
    task_param->url = source_data.source();
    task_param->save_path = path_unit::UnifiedSlash(source_data.save_path());
    task_param->file_name = source_data.file_name();
    task_param->file_size = nas::utils::StringToInt64(source_data.file_size());

    ExecCreate(task_param, context_field, &need_parse_tasks, &success_list,
               &failed_list);
  }
  AssembleMessage(TaskOperation::kCreate, context_field,
                  std::move(success_list), std::move(failed_list),
                  &message_data);
  nas::utils::Notify(kTaskCreateOperationId, std::move(message_data),
                     context_field);

  if (!need_parse_tasks.empty()) {
    ParseTasks(need_parse_tasks);
  }
}

void UserSelfMgr::CreateTorrentTask(
    const download::v1::CreateTorrentDownloadTaskRequest* request,
    const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::CreateTorrentDownloadTaskRequest>(
            *request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::CreateTorrentTask, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;
  std::vector<TaskBasePtr> need_parse_tasks;
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TorrentTaskParamPtr task_param = std::make_shared<TorrentTaskParam>();
    task_param->id = request->task_id();
    task_param->url =
        request
            ->source();  // 真正的下载源，在创建种子任务后会改变其种子文件路径
    task_param->save_path = path_unit::UnifiedSlash(request->save_path());
    task_param->file_name = request->task_name();
    task_param->is_private_cloud_source = request->is_private_cloud_source();
    task_param->front_trans_url = request->source();

    int size = request->torrent_item_select_list_size();
    for (int i = 0; i < size; ++i) {
      download_data_struct::TorrentItemSelectData item_select_data =
          request->torrent_item_select_list(i);
      TorrenFileItemSelect torrent_item_select;
      torrent_item_select.index = item_select_data.index();
      torrent_item_select.is_selected = item_select_data.is_selected();
      if (torrent_item_select.is_selected) {
        task_param->selected_count++;
      }
      task_param->torrent_item_select_list.push_back(torrent_item_select);
    }

    ExecCreate(task_param, context_field, &need_parse_tasks, &success_list,
               &failed_list);
  } while (0);

  AssembleMessage(TaskOperation::kCreate, context_field,
                  std::move(success_list), std::move(failed_list),
                  &message_data, err);
  nas::utils::Notify(kTaskCreateOperationId, std::move(message_data),
                     context_field);

  if (!need_parse_tasks.empty()) {
    ParseTasks(need_parse_tasks);
  }
}

void UserSelfMgr::Pause(const download::v1::PauseRequest* request,
                        const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::PauseRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::Pause, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kPause;

  int size = request->task_ids_size();
  for (int i = 0; i < size; ++i) {
    std::string id = request->task_ids(i);
    DownloadErrorCode err = ExecPauseOperate(id);
    BuildMessageData(id, err, operation, &success_list, &failed_list);
  }
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::PauseAll(const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&UserSelfMgr::PauseAll, shared_from_this(),
                                  context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;
  TaskOperation operation = TaskOperation::kPauseAll;
  for (const auto& [id, task] : task_list_) {
    TaskStatus status = task->GetTaskStatus();
    if (status == TaskStatus::kDownloading || status == TaskStatus::kWaitting ||
        status == TaskStatus::kParsing) {
      DownloadErrorCode err = ExecPauseOperate(id);
      BuildMessageData(id, err, operation, &success_list, &failed_list);
    }
  }

  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::Resume(const download::v1::ResumeRequest* request,
                         const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::ResumeRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::Resume, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kResume;

  std::vector<TaskBasePtr> create_time_task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);

  int size = request->task_ids_size();
  for (const auto& task : create_time_task_queue) {
    std::string task_id = task->GetTaskParam()->id;
    for (int i = 0; i < size; ++i) {
      std::string id = request->task_ids(i);
      if (id != task_id) {
        continue;
      }
      DownloadErrorCode err = ExecResumeOperate(task_id);
      BuildMessageData(id, err, operation, &success_list, &failed_list);
    }
  }
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::ResumeAll(const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&UserSelfMgr::ResumeAll, shared_from_this(),
                                  context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;
  TaskOperation operation = TaskOperation::kResumeAll;

  std::vector<TaskBasePtr> create_time_task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);

  for (const auto& task : create_time_task_queue) {
    TaskStatus status = task->GetTaskStatus();
    std::string name = task->GetTaskParam()->file_name;
    if (status == TaskStatus::kPaused || status == TaskStatus::kFailed) {
      std::string task_id = task->GetTaskParam()->id;
      DownloadErrorCode err = ExecResumeOperate(task_id);
      BuildMessageData(task_id, err, operation, &success_list, &failed_list);
    }
  }
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::Delete(const download::v1::DeleteRequest* request,
                         const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::DeleteRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::Delete, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kDelete;

  int size = request->task_ids_size();
  for (int i = 0; i < size; ++i) {
    std::string id = request->task_ids(i);
    ExecDeleteOperate(id, request->is_delete_local_file(), false, &success_list,
                      &failed_list);
  }
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::DeleteCategory(
    const download::v1::DeleteCategoryRequest* request,
    const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::DeleteCategoryRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::DeleteCategory, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kDeleteCategory;

  nas::TaskCategory task_category = nas::TaskCategory(request->task_category());
  ExecDeleteCategoryOperate(task_category, request->is_delete_local_file(),
                            &success_list, &failed_list);
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::Retry(const download::v1::RetryRequest* request,
                        const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::RetryRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::Retry, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kRetry;

  int size = request->task_ids_size();
  std::vector<TaskBasePtr> create_time_task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);

  for (const auto& task : create_time_task_queue) {
    std::string task_id = task->GetTaskParam()->id;
    for (int i = 0; i < size; ++i) {
      std::string id = request->task_ids(i);
      if (id != task_id) {
        continue;
      }
      DownloadErrorCode err = ExecRetryOperate(task_id);
      BuildMessageData(task_id, err, operation, &success_list, &failed_list);
    }
  }

  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::RetryAll(const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&UserSelfMgr::RetryAll, shared_from_this(),
                                  context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;
  TaskOperation operation = TaskOperation::kRetryAll;

  std::vector<TaskBasePtr> create_time_task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);
  for (const auto& task : create_time_task_queue) {
    if (task->GetTaskStatus() == TaskStatus::kFailed ||
        !task->GetTaskInfo()->IsValid()) {
      std::string task_id = task->GetTaskParam()->id;
      DownloadErrorCode err = ExecRetryOperate(task_id);
      BuildMessageData(task_id, err, operation, &success_list, &failed_list);
    }
  }

  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::Clean(const download::v1::CleanRequest* request,
                        const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request = std::make_unique<download::v1::CleanRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::Clean, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kClean;

  int size = request->task_ids_size();
  for (int i = 0; i < size; ++i) {
    std::string id = request->task_ids(i);
    ExecDeleteOperate(id, false, true, &success_list, &failed_list);
  }
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::CleanCategory(
    const download::v1::CleanCategoryRequest* request,
    const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::CleanCategoryRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::CleanCategory, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kCleanCategory;

  nas::TaskCategory task_category = nas::TaskCategory(request->task_category());
  ExecCleanCategoryOperate(task_category, &success_list, &failed_list);

  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::Restore(const download::v1::RestoreRequest* request,
                          const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    auto copy_request =
        std::make_unique<download::v1::RestoreRequest>(*request);
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::Restore, shared_from_this(),
                       base::Owned(copy_request.release()), context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kRestore;

  std::vector<TaskBasePtr> create_time_task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);

  int size = request->task_ids_size();
  for (const auto& task : create_time_task_queue) {
    for (int i = 0; i < size; ++i) {
      std::string id = request->task_ids(i);
      if (id != task->GetTaskParam()->id) {
        continue;
      }

      if (task->GetTaskStatus() != TaskStatus::kDeleted) {
        continue;
      }

      DownloadErrorCode err = ExecRestoreOperate(id);
      BuildMessageData(id, err, operation, &success_list, &failed_list);
    }
  }
  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::RestoreAll(const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&UserSelfMgr::RestoreAll, shared_from_this(),
                                  context_field));
    return;
  }

  base::Value::Dict message_data;
  base::Value::List success_list;
  base::Value::List failed_list;

  TaskOperation operation = TaskOperation::kRestoreAll;
  std::vector<TaskBasePtr> create_time_task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);

  for (const auto& task : create_time_task_queue) {
    if (task->GetTaskStatus() != TaskStatus::kDeleted) {
      continue;
    }
    std::string task_id = task->GetTaskParam()->id;
    DownloadErrorCode err = ExecRestoreOperate(task_id);
    BuildMessageData(task_id, err, operation, &success_list, &failed_list);
  }

  AssembleMessage(operation, context_field, std::move(success_list),
                  std::move(failed_list), &message_data);
  nas::utils::Notify(kTaskControlOperationId, std::move(message_data),
                     context_field);
}

void UserSelfMgr::GetTaskInfoList(const ContextField& context_field,
                                  GetTaskListCallback callback) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::GetTaskInfoList, shared_from_this(),
                       context_field, std::move(callback)));
    return;
  }

  if (user_info_) {
    user_info_->SetContextField(context_field);
  }

  std::vector<TaskInfoPtr> task_info_list;
  for (const auto& [task_id, task] : task_list_) {
    TaskInfoPtr task_info = task->GetTaskInfo();
    task_info_list.push_back(task_info);
    task->SetContextField(context_field);
  }
  if (!callback.is_null()) {
    std::move(callback).Run(task_info_list);
  }
}

void UserSelfMgr::UpdateUserInfo(const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(FROM_HERE,
                           base::BindOnce(&UserSelfMgr::UpdateUserInfo,
                                          shared_from_this(), context_field));
    return;
  }
  if (user_info_) {
    user_info_->SetContextField(context_field);
  }
}

void UserSelfMgr::SetUserInfo(const UserInfoPtr user_info) {
  user_info_ = user_info;
}

void UserSelfMgr::TimerChcek() {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(FROM_HERE, base::BindOnce(&UserSelfMgr::TimerChcek,
                                                     shared_from_this()));
    return;
  }

  if (!user_info_) {
    return;
  }

  // 计算统计数据
  DownloadStatisticsInfo statistics_info;

  // 1、定时检查是否有小于限制数量的任务，如果有就设置状态为进行中，然后开始任务；
  // 2、获取任务信息
  base::Value::Dict tasks_json_data;
  CheckTaskAndGetTaskInfo(&statistics_info, &tasks_json_data);
  ContextField context_field = user_info_->GetContextField();
  if (context_field.nas_token().empty() || context_field.user_id().empty()) {
    LOG(WARNING) << __func__ << ": nas_token is empty or user id is empty";
    return;
  }

  if (tasks_json_data.empty()) {
    LOG(WARNING) << __func__
                  << ": task json data is empty, so do not publish message";
    return;
  }

  nas::utils::Notify(kTaskProgressInfoCallId, std::move(tasks_json_data),
                     context_field);
}

void UserSelfMgr::ResumeFromDb(const std::vector<TaskInfoPtr>& task_info_list,
                               const ContextField& context_field) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&UserSelfMgr::ResumeFromDb, shared_from_this(),
                       task_info_list, context_field));
    return;
  }

  for (TaskInfoPtr task_info : task_info_list) {
    if (!task_info) {
      continue;
    }

    TaskBasePtr task =
        TaskFactory::CreateTask(task_info->GetTaskParam(), context_field);
    if (!task) {
      LOG(ERROR) << "resume task failed, task is null, task_id is "
                 << task_info->GetTaskParam()->id;
      continue;
    }
    task->ResumeDataFromDb(task_info);
    AddTask(task_info->GetTaskParam()->id, task);
  }
}

void UserSelfMgr::ExecCreate(const TaskParamPtr task_param,
                             const ContextField& context_field,
                             std::vector<TaskBasePtr>* need_parse_task_list,
                             base::Value::List* success_list,
                             base::Value::List* failed_list) {
  DownloadErrorCode err_code = DownloadErrorCode::kNormal;
  std::string same_task_id;
  TaskBasePtr task = nullptr;
  do {
    // 创建任务
    err_code = CreateTask(task_param, context_field, task, same_task_id);
    if (err_code != DownloadErrorCode::kNormal) {
      break;
    }

    DownloadType download_type = task->GetDownloadType();
    switch (download_type) {
      case DownloadType::kMagnet:
        err_code = ExecMagnet(task);
        break;
      case DownloadType::kTorrent:
        err_code = ExecTorrent(task);
      default:
        err_code = ExecNoneBt(task);
        break;
    }

    if (err_code != DownloadErrorCode::kNormal) {
      break;
    }

    if (!AddTask(task_param->id, task)) {
      task->Cancel(true);
      err_code = DownloadErrorCode::kAddTaskFailed;
      break;
    }

    need_parse_task_list->push_back(task);
  } while (0);

  // 删除种子文件
  if (err_code != DownloadErrorCode::kNormal &&
      TaskFactory::GetDownloadType(task_param->url) == DownloadType::kTorrent) {
    TorrentTaskParamPtr torrent_param =
        std::static_pointer_cast<TorrentTaskParam>(task_param);
    if (task && torrent_param) {
      if (err_code == DownloadErrorCode::kFileNameExisted) {
        nas::utils::DeleteFile(torrent_param->url);
      } else {
        nas::utils::DeleteFile(torrent_param->url);
        if (!torrent_param->is_private_cloud_source) {
          nas::utils::DeleteFile(torrent_param->front_trans_url);
        }
      }
    }
  }

  BuildCreateMessage(err_code, task_param->id, same_task_id, success_list,
                     failed_list);
}

DownloadErrorCode UserSelfMgr::CreateTask(const TaskParamPtr task_param,
                                          const ContextField& context_field,
                                          TaskBasePtr& task,
                                          std::string& same_task_id) {
  DownloadErrorCode err_code = DownloadErrorCode::kNormal;
  do {
    if (!nas::utils::IsValidFilename(task_param->file_name)) {
      err_code = DownloadErrorCode::kInvalidFileName;
      break;
    }

    // 如果保存路径不存在，则创建一个保存路径
    err_code = CreateSavePath(task_param->save_path);
    if (err_code != DownloadErrorCode::kNormal) {
      break;
    }

    // 是否是受保护目录
    err_code = IsProtectedPath(task_param->save_path);
    if (err_code != DownloadErrorCode::kNormal) {
      break;
    }

    // 创建一个临时文件，判断是否具有写入权限
    err_code = HasWriteAccess(task_param->save_path);
    if (err_code != DownloadErrorCode::kNormal) {
      break;
    }

    // 查找是否具有相同的任务id
    if (FindTask(task_param->id)) {
      LOG(ERROR) << "task_id has existed, task_id =  " << task_param->id;
      err_code = DownloadErrorCode::kTaskIdExisted;
      break;
    }
    task = TaskFactory::CreateTask(task_param, context_field);
    if (!task) {
      LOG(ERROR) << "create task failed, task_id =  " << task_param->id;
      err_code = DownloadErrorCode::kCreateTaskFailed;
      break;
    }

    // 是否存在同样的链接
    same_task_id = IsTaskExisted(task_param);
    if (!same_task_id.empty()) {
      err_code = DownloadErrorCode::kExistSameTask;
      LOG(WARNING) << "exist same task id =  " << same_task_id;
      break;
    }

    // 条件判断
    err_code = task->CheckCondition();
    if (err_code != DownloadErrorCode::kNormal) {
      LOG(ERROR) << "task conditon check failed, task_id =  " << task_param->id;
      break;
    }

    // 种子任务判断保存路径中是否存在相同任务名
    DownloadType download_type = TaskFactory::GetDownloadType(task_param->url);
    if (download_type == kTorrent || download_type == kMagnet) {
      if (!task_param->file_name.empty()) {
        base::FilePath file_path = nas::path_unit::BaseFilePathFromU8(
            task_param->save_path + "/" + task_param->file_name);
        if (base::PathExists(file_path)) {
          err_code = DownloadErrorCode::kFileNameExisted;
          LOG(ERROR) << "file name is already exists! file_name: " << file_path;
          break;
        }
      }
    }

    if (user_info_) {
      user_info_->SetContextField(context_field);
    }

  } while (0);

  return err_code;
}

bool UserSelfMgr::AddTask(const std::string& task_id, TaskBasePtr task) {
  auto iter = task_list_.find(task_id);
  if (iter != task_list_.end()) {
    return false;
  }

  task_list_[task_id] = task;

  // 向数据库添加数据，从数据库恢复时token为空不需要再添加进数据库
  if (user_info_) {
    ContextField context_field = user_info_->GetContextField();
    TaskParamPtr param = task->GetTaskParam();
    if (!task->IsResumeFromDb(param)) {
      DownloadStoreHelper::GetInstance()->AddTaskInfo(context_field.user_id(),
                                                      task->GetTaskInfo());
    }
  }
  TaskRefCount::GetInstance()->AddRef(task->GetTaskHash());
  return true;
}

TaskBasePtr UserSelfMgr::FindTask(const std::string& task_id) {
  auto iter = task_list_.find(task_id);
  if (iter == task_list_.end()) {
    return nullptr;
  }

  return iter->second;
}

void UserSelfMgr::AssembleMessage(const TaskOperation& operation,
                                  const ContextField& context_field,
                                  base::Value::List success_list,
                                  base::Value::List failed_list,
                                  base::Value::Dict* message_data,
                                  DownloadErrorCode error) {
  if (failed_list.size() > 0) {
    switch (operation) {
      case TaskOperation::kCreate:
        if (error == DownloadErrorCode::kNormal) {
          error = DownloadErrorCode::kCreateTaskFailed;
        }
        break;
      case TaskOperation::kPause:
      case TaskOperation::kPauseAll:
        error = DownloadErrorCode::kPauseTaskFailed;
        break;
      case TaskOperation::kResume:
      case TaskOperation::kResumeAll:
        error = DownloadErrorCode::kResumeTaskFailed;
        break;
      case TaskOperation::kDelete:
      case TaskOperation::kDeleteCategory:
        error = DownloadErrorCode::kDeleteTaskFailed;
        break;
      case TaskOperation::kRetry:
      case TaskOperation::kRetryAll:
        error = DownloadErrorCode::kRetryTaskFailed;
        break;
      case TaskOperation::kClean:
      case TaskOperation::kCleanCategory:
        error = DownloadErrorCode::kCleanTaskFailed;
        break;
      case TaskOperation::kRestore:
      case TaskOperation::kRestoreAll:
        error = DownloadErrorCode::kRestoreTaskFailed;
        break;
      default:
        break;
    }
  }
  base::Value::Dict msg_dict;
  msg_dict.Set("action", GetAction(operation));
  msg_dict.Set("success_list", std::move(success_list));
  msg_dict.Set("failed_list", std::move(failed_list));
  msg_dict.Set("operation_id", context_field.call_id());
  message_data->Set("data", std::move(msg_dict));
  message_data->Set("errno", nas::utils::DownloadErrNoToOutErrNo(error));
  message_data->Set("msg", nas::GetDownloadErrorDesc(error));
}

std::string UserSelfMgr::GetAction(const TaskOperation& operation) {
  std::string action;
  switch (operation) {
    case TaskOperation::kCreate:
      action = "create";
      break;
    case TaskOperation::kPause:
      action = "pause";
      break;
    case TaskOperation::kPauseAll:
      action = "pause_all";
      break;
    case TaskOperation::kResume:
      action = "resume";
      break;
    case TaskOperation::kResumeAll:
      action = "resume_all";
      break;
    case TaskOperation::kDelete:
      action = "delete";
      break;
    case TaskOperation::kDeleteCategory:
      action = "delete_category";
      break;
    case TaskOperation::kRetry:
      action = "retry";
      break;
    case TaskOperation::kRetryAll:
      action = "retry_all";
      break;
    case TaskOperation::kClean:
      action = "clean";
      break;
    case TaskOperation::kCleanCategory:
      action = "clean_category";
      break;
    case TaskOperation::kRestore:
      action = "restore";
      break;
    case TaskOperation::kRestoreAll:
      action = "restore_all";
      break;
    default:
      break;
  }
  return action;
}

void UserSelfMgr::ParseTasks(const std::vector<TaskBasePtr>& tasks) {
  for (TaskBasePtr task : tasks) {
    if (!task) {
      LOG(ERROR) << "parse task is null;";
      continue;
    }
    task->Parse();
  }
}

void UserSelfMgr::BuildCreateMessage(DownloadErrorCode error,
                                     const std::string& task_id,
                                     const std::string& same_task_id,
                                     base::Value::List* success_list,
                                     base::Value::List* failed_list) {
  if (error) {
    base::Value::Dict json_data;
    json_data.Set("task_id", task_id);
    json_data.Set("same_task_id", same_task_id);
    json_data.Set("err_code", nas::utils::DownloadErrNoToOutErrNo(error));
    json_data.Set("msg", nas::GetDownloadErrorDesc(error));
    failed_list->Append(std::move(json_data));
  } else {
    success_list->Append(task_id);
  }
}

DownloadErrorCode UserSelfMgr::ExecPauseOperate(const std::string& task_id) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TaskBasePtr task = FindTask(task_id);
    if (!task) {
      err = DownloadErrorCode::kTaskIdIsNotExist;
      break;
    }
    SetTaskContextField(task);
    err = PauseTask(task);
  } while (0);

  return err;
}

DownloadErrorCode UserSelfMgr::ExecResumeOperate(const std::string& task_id) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TaskBasePtr task = FindTask(task_id);
    if (!task) {
      err = DownloadErrorCode::kTaskIdIsNotExist;
      break;
    }
    SetTaskContextField(task);
    DownloadType type = task->GetDownloadType();
    bool ret = true;
    if (task->GetTaskStatus() == TaskStatus::kPaused) {
      // 如果是磁力链接，判断是否解析完成
      if (type == DownloadType::kMagnet) {
        if (task->IsParseFinish()) {
          task->PublishTaskBaseInfo();
        } else {
          task->SetTaskStatus(TaskStatus::kParsing);
          break;
        }
      }
      err = TryResume(task);
    } else if (task->GetTaskStatus() == TaskStatus::kFailed) {
      if (!task->Retry()) {
        break;
      }
      err = TryResume(task);
    } else if ((type == DownloadType::kTorrent ||
                type == DownloadType::kMagnet) &&
               task->GetTaskStatus() == TaskStatus::kFinished) {
      ret = task->Resume();
      if (ret) {
        task->SetTaskStatus(TaskStatus::kSeeding);
      }
      err = ret ? DownloadErrorCode::kNormal
                : DownloadErrorCode::kResumeTaskFailed;
    } else {
      err = DownloadErrorCode::kResumeTaskFailed;
    }
  } while (0);

  return err;
}

DownloadErrorCode UserSelfMgr::ExecDeleteOperate(
    const std::string& task_id,
    bool is_delete_local_file,
    bool is_judge_task_valid,
    base::Value::List* success_list,
    base::Value::List* failed_list) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TaskBasePtr task = FindTask(task_id);
    if (!task) {
      err = DownloadErrorCode::kTaskIdIsNotExist;
      break;
    }
    SetTaskContextField(task);
    if (is_judge_task_valid) {
      if (task->GetTaskInfo()->IsValid()) {
        err = DownloadErrorCode::kCleanTaskFailed;
        break;
      }
    }

    if (task->GetTaskCategory() == nas::TaskCategory::kRecyclebinTask) {
      if (is_delete_local_file && task->IsDownloadFinished() &&
          !task->DeleteLocalFile()) {
        err = DownloadErrorCode::kDeleteLocalFileFailed;
        break;
      }

      LOG(INFO) << "Cancel task:" << task_id;
      if (!task->Cancel(is_delete_local_file)) {
        LOG(ERROR) << "delete task failed! task_id: " << task_id;
        err = DownloadErrorCode::kDeleteTaskFailed;
        break;
      }

      DownloadStoreHelper::GetInstance()->DeleteTaskInfo(user_id_, task_id);
      success_list->Append(task->BuildMessageData(TaskOperation::kDelete));
      task_list_.erase(task_id);
      LOG(INFO) << "erase task id = " << task_id;
    } else {
      if (!task->DeleteToRecycleBin()) {
        err = DownloadErrorCode::kDeleteTaskFailed;
      } else {
        success_list->Append(task->BuildMessageData(TaskOperation::kDelete));
      }
    }
  } while (0);

  if (err) {
    nas::CallResult call_result = utils::ErrNoToCallResult(err);
    base::Value::Dict dict;
    dict.Set("task_id", task_id);
    dict.Set("err_code", static_cast<int>(call_result.errcode));
    dict.Set("msg", call_result.msg);
    failed_list->Append(std::move(dict));
  }

  return err;
}

// void UserSelfMgr::ExecPauseAllOperate(base::Value::List* success_list,
//                                       base::Value::List* failed_list) {
//   TaskOperation operation = TaskOperation::kPauseAll;
//   for (const auto& [task_id, task] : task_list_) {
//     SetTaskContextField(task);
//     TaskStatus status = task->GetTaskStatus();
//     if (status == TaskStatus::kDownloading || status == TaskStatus::kWaitting
//     ||
//         status == TaskStatus::kParsing) {
//       DownloadErrorCode err = PauseTask(task);
//       BuildMessageData(task_id, err, operation, success_list, failed_list);
//     }
//   }
// }

void UserSelfMgr::ExecDeleteCategoryOperate(nas::TaskCategory task_category,
                                            bool is_delete_local_file,
                                            base::Value::List* success_list,
                                            base::Value::List* failed_list) {
  TaskOperation operation = TaskOperation::kDeleteCategory;
  for (auto iter = task_list_.begin(); iter != task_list_.end();) {
    TaskBasePtr task = iter->second;
    std::string task_id = iter->first;
    SetTaskContextField(task);
    if (task_category == nas::TaskCategory::kAll) {
      LOG(WARNING) << "delete task_id: " << task_id
                   << ", user_id: " << user_id_;
      if (!task->Cancel(true)) {
        LOG(ERROR) << "delete task failed! task_id: " << task_id;
        BuildMessageData(task_id, DownloadErrorCode::kDeleteTaskFailed,
                         operation, success_list, failed_list);
        iter++;
      } else {
        DownloadStoreHelper::GetInstance()->DeleteTaskInfo(user_id_, task_id);
        BuildMessageData(task_id, DownloadErrorCode::kNormal, operation,
                         success_list, failed_list);
        iter = task_list_.erase(iter);
        LOG(INFO) << "erase task id = " << task_id;
      }
      continue;
    }

    nas::TaskCategory current_category = task->GetTaskCategory();
    if (current_category != task_category) {
      iter++;
      continue;
    }

    if (task_category == nas::TaskCategory::kRecyclebinTask) {
      if (is_delete_local_file && task->IsDownloadFinished() &&
          !task->DeleteLocalFile()) {
        BuildMessageData(task_id, DownloadErrorCode::kDeleteLocalFileFailed,
                         operation, success_list, failed_list);
        iter++;
        continue;
      }

      if (!task->Cancel(is_delete_local_file)) {
        LOG(ERROR) << "delete task failed! task_id: " << task_id;
        BuildMessageData(task_id, DownloadErrorCode::kDeleteTaskFailed,
                         operation, success_list, failed_list);
        iter++;
        continue;
      }

      DownloadStoreHelper::GetInstance()->DeleteTaskInfo(user_id_, task_id);
      BuildMessageData(task_id, DownloadErrorCode::kNormal, operation,
                       success_list, failed_list);
      iter = task_list_.erase(iter);
      LOG(INFO) << "erase task id = " << task_id;
    } else {
      task->DeleteToRecycleBin();
      iter++;
      BuildMessageData(task_id, DownloadErrorCode::kNormal, operation,
                       success_list, failed_list);
    }
  }
}

DownloadErrorCode UserSelfMgr::ExecRetryOperate(const std::string& task_id) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TaskBasePtr task = FindTask(task_id);
    if (!task) {
      err = DownloadErrorCode::kTaskIdIsNotExist;
      break;
    }
    SetTaskContextField(task);
    if (task->GetTaskStatus() == TaskStatus::kFailed ||
        !task->GetTaskInfo()->IsValid()) {
      bool ret = task->Retry();
      if (!ret) {
        task->GetTaskInfo()->SetIsValid(true);
        break;
      }
      err = TryResume(task);
      task->GetTaskInfo()->SetIsValid(true);
    }

  } while (0);

  return err;
}

void UserSelfMgr::ExecCleanCategoryOperate(nas::TaskCategory task_category,
                                           base::Value::List* success_list,
                                           base::Value::List* failed_list) {
  TaskOperation operation = TaskOperation::kCleanCategory;
  for (auto iter = task_list_.begin(); iter != task_list_.end();) {
    TaskBasePtr task = iter->second;
    std::string task_id = iter->first;
    SetTaskContextField(task);
    bool ret = true;

    if (task->GetTaskCategory() != task_category ||
        task->GetTaskInfo()->IsValid()) {
      iter++;
      continue;
    }

    if (task_category == nas::TaskCategory::kRecyclebinTask) {
      ret = task->Cancel(true);
      if (!ret) {
        LOG(ERROR) << "clean task failed! task_id: " << task_id;
        BuildMessageData(task_id, DownloadErrorCode::kDeleteTaskFailed,
                         operation, success_list, failed_list);
        iter++;
        continue;
      }

      DownloadStoreHelper::GetInstance()->DeleteTaskInfo(user_id_, task_id);
      BuildMessageData(task_id, DownloadErrorCode::kNormal, operation,
                       success_list, failed_list);
      iter = task_list_.erase(iter);
      LOG(INFO) << "clean operate: erase task id = " << task_id;
    } else {
      task->DeleteToRecycleBin();
      iter++;
      BuildMessageData(task_id, DownloadErrorCode::kNormal, operation,
                       success_list, failed_list);
    }
  }
}

DownloadErrorCode UserSelfMgr::ExecRestoreOperate(const std::string& task_id) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  do {
    TaskBasePtr task = FindTask(task_id);
    if (!task) {
      err = DownloadErrorCode::kTaskIdIsNotExist;
      break;
    }
    SetTaskContextField(task);
    if (task->GetTaskInfo()->IsValid()) {
      err = task->Restore() ? DownloadErrorCode::kNormal
                            : DownloadErrorCode::kRestoreTaskFailed;
      TaskStatus status = task->GetTaskStatus();
      if (status == TaskStatus::kWaitting && IsCanStart(task)) {
        if (task->IsStarted()) {
          task->Resume();
        } else {
          err = task->Start();
        }
      }
    } else {
      err = ExecRetryOperate(task_id);
    }
  } while (0);

  return err;
}

std::string UserSelfMgr::IsTaskExisted(const TaskParamPtr task_param) {
  std::string same_id;
  for (const auto& [task_id, task] : task_list_) {
    TaskParamPtr param = task->GetTaskParam();
    if (param->url == task_param->url ||
        (!task_param->hash.empty() && !param->hash.empty() &&
         task_param->hash == param->hash)) {
      same_id = param->id;
      break;
    }
  }
  return same_id;
}

DownloadErrorCode UserSelfMgr::PauseTask(TaskBasePtr task) {
  bool ret = true;
  TaskStatus status = task->GetTaskStatus();
  if (status == TaskStatus::kDownloading) {
    ret = task->Pause();
  } else if (status == TaskStatus::kWaitting ||
             status == TaskStatus::kParsing) {
    task->SetTaskStatus(TaskStatus::kPaused);
  } else if (status == TaskStatus::kSeeding) {
    ret = task->Pause();
    if (ret) {
      task->SetTaskStatus(TaskStatus::kFinished);
    }
  } else {
    ret = false;
  }

  return ret ? DownloadErrorCode::kNormal : DownloadErrorCode::kPauseTaskFailed;
}

bool UserSelfMgr::IsCanStart(TaskBasePtr task) {
  int downloading_task_num = 0;
  for (const auto& [id, task] : task_list_) {
    if (task->GetTaskStatus() == TaskStatus::kDownloading) {
      downloading_task_num++;
    }
  }

  int task_limit_count = 5;
  if (user_info_) {
    task_limit_count = user_info_->GetTaskLimitCount();
  }

  if (downloading_task_num >= task_limit_count) {
    LOG(WARNING) << "current downloading task num: " << downloading_task_num
                 << ", more than task_limit_count: " << task_limit_count;
    return false;
  }

  return true;
}

std::vector<TaskBasePtr> UserSelfMgr::SortTask(nas::SortStyle sort_style,
                                               bool is_ascending) {
  std::vector<TaskBasePtr> task_queue;
  for (const auto& entry : task_list_) {
    task_queue.push_back(entry.second);
  }

  if (user_info_ && user_info_->IsPrioritySmallLimitTask()) {
    comparator_.SetThreshold(user_info_->GetSmallTaskSize() * 1024);
    sort(task_queue.begin(), task_queue.end(), comparator_);
  } else {
    PfnOrder order = GetOrderFunc(sort_style, is_ascending);
    sort(task_queue.begin(), task_queue.end(), order);
  }

  return task_queue;
}

PfnOrder UserSelfMgr::GetOrderFunc(nas::SortStyle style, bool asc) {
  PfnOrder order = AscendingByCreateTime;
  if (asc) {
    switch (style) {
      case nas::SortStyle::kCreateTime:
        order = AscendingByCreateTime;
        break;
      case nas::SortStyle::kTaskSize:
        order = AscendingByTaskSise;
        break;

      default:
        break;
    }
  } else {
    switch (style) {
      case nas::SortStyle::kCreateTime:
        order = DecendingByCreateTime;
        break;
      default:
        break;
    }
  }

  return order;
}

void UserSelfMgr::BuildMessageData(const std::string& task_id,
                                   DownloadErrorCode err,
                                   const TaskOperation& operation,
                                   base::Value::List* success_list,
                                   base::Value::List* failed_list) {
  auto iter = task_list_.find(task_id);
  if (iter == task_list_.end()) {
    return;
  }
  TaskBasePtr task = iter->second;
  if (!task) {
    err = DownloadErrorCode::kTaskIdIsNotExist;
  }
  if (err) {
    nas::CallResult call_result = utils::ErrNoToCallResult(err);
    base::Value::Dict dict;
    dict.Set("task_id", task_id);
    dict.Set("task_status", task->GetTaskStatus());
    dict.Set("task_category", task->GetTaskCategory());
    dict.Set("err_code", static_cast<int>(call_result.errcode));
    dict.Set("msg", call_result.msg);
    failed_list->Append(std::move(dict));
  } else {
    success_list->Append(task->BuildMessageData(operation));
  }
}

void UserSelfMgr::CheckTaskAndGetTaskInfo(
    DownloadStatisticsInfo* statistics_info,
    base::Value::Dict* msg) {
  std::vector<TaskBasePtr> task_queue =
      SortTask(nas::SortStyle::kCreateTime, true);

  base::Value::List downloading_json_list;
  base::Value::List seeding_json_list;

  bool is_reach = CheckTaskCountLimit(task_queue);

  for (auto& task : task_queue) {
    // 根据任务状态及错误码来判断任务失败情况，任务失败后发送一次失败消息,
    // 主要为了解决重启服务以及下载过程中遇到下载失败的情况能够正常推送一次失败消息
    if (!task->CheckTaskIsFailed()) {
      continue;
    }
    TaskStatus status = task->GetTaskStatus();
    TaskProgressInfoPtr progress_info = task->GetTaskInfo()->GetProgressInfo();
    switch (status) {
      case TaskStatus::kPaused:
      case TaskStatus::kWaitting: {
        base::Value::Dict progress_result = task->BuildDownloadingData();
        downloading_json_list.Append(std::move(progress_result));
      } break;
      case TaskStatus::kChecking:
      case TaskStatus::kDownloading: {
        base::Value::Dict progress_result;
        task->UpdateProgressInfo(&progress_result);
        downloading_json_list.Append(std::move(progress_result));
        statistics_info->download_rate += progress_info->GetDownloadSpeed();
      } break;
      case TaskStatus::kSeeding:
        if (task->GetDownloadType() == DownloadType::kMagnet ||
            task->GetDownloadType() == DownloadType::kTorrent) {
          BtTaskBasePtr bt_task = std::static_pointer_cast<BtTaskBase>(task);
          if (bt_task) {
            progress_info->SetUploadSpeed(bt_task->GetUploadRate());
            // 达到结束做种条件后就不用在seeding中展示了
            if (bt_task->CheckSeedingLimits()) {
              break;
            }
            base::Value::Dict seeding_json;
            bt_task->UpdateSeedingInfo(&seeding_json);
            if (seeding_json.size() > 0) {
              seeding_json_list.Append(std::move(seeding_json));
            }
          }
        }
      case TaskStatus::kFinished:
      case TaskStatus::kDeleted:
        CheckInvalidTask(task);
        break;
      default:
        break;
    }
    statistics_info->upload_rate += progress_info->GetUploadSpeed();
  }

  DownloadStatisticsInfo store_statistics =
      user_info_->GetDownloadStatisticsInfo();
  // 达到一定条件再发送消息，没必要一直发送，节省流量
  if (is_reach || seeding_json_list.size() > 0 ||
      statistics_info->download_rate > 0 || statistics_info->upload_rate > 0 ||
      (store_statistics.download_rate > 0 &&
       statistics_info->download_rate <= 0) ||
      (store_statistics.upload_rate > 0 && statistics_info->upload_rate <= 0)) {
    user_info_->SetDownloadStatisticsInfo(*statistics_info);
    base::Value::Dict statistics_json_dict;
    statistics_json_dict.Set("download_speed", statistics_info->download_rate);
    statistics_json_dict.Set("upload_speed", statistics_info->upload_rate);

    base::Value::Dict json_data;
    json_data.Set("downloading", std::move(downloading_json_list));
    json_data.Set("seeding", std::move(seeding_json_list));
    json_data.Set("statistics", std::move(statistics_json_dict));
    msg->Set("data", std::move(json_data));
  }
}

void UserSelfMgr::CheckInvalidTask(TaskBasePtr task) {
  if (!task->IsStarted()) {
    return;
  }

  TaskInfoPtr task_info = task->GetTaskInfo();
  if (task_info->IsValid() && !task->Valid()) {
    task->PublishInvalidTaskInfo();
    task_info->SetIsValid(false);
  }
}

void UserSelfMgr::UpdateStatisticsInfo(
    base::Value::Dict* statistics_json_dict) {}

NasThread* UserSelfMgr::GetThread() const {
  NasThread* user_thread = NasThread::GetThread(kUserThreadName);
  DCHECK(user_thread);
  return user_thread;
}

void UserSelfMgr::OnParseComplete(const std::string& task_id,
                                  const std::string& result) {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(FROM_HERE,
                           base::BindOnce(&UserSelfMgr::OnParseComplete,
                                          shared_from_this(), task_id, result));
    return;
  }

  LOG(INFO) << "on parse completed: task_id = " << task_id
            << ", result = " << result;
  TaskBasePtr task = FindTask(task_id);
  BtMagnetTaskPtr magnet_task = std::static_pointer_cast<BtMagnetTask>(task);
  if (magnet_task) {
    magnet_task->SetParseFinish(true);
    magnet_task->UpdateFileBaseInfo(result);
    magnet_task->PublishTaskBaseInfo();

    if (IsCanStart(magnet_task)) {
      if (!SystemCloudConfig::GetInstance()->IsBtAutoDownload()) {
        magnet_task->Pause();
        magnet_task->SetTaskStatus(TaskStatus::kPaused);
      } else {
        magnet_task->SetTaskStatus(TaskStatus::kDownloading);
      }
    } else {
      magnet_task->Pause();
      magnet_task->SetTaskStatus(TaskStatus::kWaitting);
    }
  }
}

bool UserSelfMgr::CheckTaskCountLimit(std::vector<TaskBasePtr>& task_queue) {
  int downloading_count = 0;
  int task_limit_count = 5;
  if (user_info_) {
    task_limit_count = user_info_->GetTaskLimitCount();
  }

  bool is_reach = false;
  for (auto& task : task_queue) {
    DownloadErrorCode err = DownloadErrorCode::kNormal;
    TaskStatus status = task->GetTaskStatus();
    if (status == TaskStatus::kDownloading) {
      downloading_count++;
      if (downloading_count > task_limit_count) {
        task->Pause();
        task->SetTaskStatus(TaskStatus::kWaitting);
        is_reach = true;
      }
    } else if (status == TaskStatus::kWaitting && IsCanStart(task)) {
      is_reach = true;
      if (task->IsStarted()) {
        task->Resume();
      } else {
        err = task->Start();
      }

      if (err != DownloadErrorCode::kNormal) {
        task->GetTaskInfo()->GetProgressInfo()->SetErrorCode(err);
      }
    }
  }
  return is_reach;
}

void UserSelfMgr::StartTimer() {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(FROM_HERE, base::BindOnce(&UserSelfMgr::StartTimer,
                                                     shared_from_this()));
    return;
  }

  if (!publish_timer_) {
    publish_timer_ = std::make_shared<base::RepeatingTimer>();
  }

  if (publish_timer_ && !publish_timer_->IsRunning()) {
    publish_timer_->Start(
        FROM_HERE, base::Seconds(1),
        base::BindRepeating(&UserSelfMgr::TimerChcek, shared_from_this()));
  }
}

void UserSelfMgr::StopTimer() {
  if (!task_runner_->RunsTasksInCurrentSequence()) {
    task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&UserSelfMgr::StopTimer, shared_from_this()));
    return;
  }

  if (publish_timer_) {
    publish_timer_->Stop();
    publish_timer_.reset();
  }
}

void UserSelfMgr::SetTaskContextField(TaskBasePtr task) {
  if (user_info_) {
    task->SetContextField(user_info_->GetContextField());
  }
}

DownloadErrorCode UserSelfMgr::TryResume(TaskBasePtr task) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  if (IsCanStart(task)) {
    if (task->IsStarted()) {
      bool ret = task->Resume();
      err = ret ? DownloadErrorCode::kNormal
                : DownloadErrorCode::kResumeTaskFailed;
    } else {
      err = task->Start();
    }
  } else {
    task->SetTaskStatus(TaskStatus::kWaitting);
  }

  return err;
}

DownloadErrorCode UserSelfMgr::ExecMagnet(TaskBasePtr task) {
  // 磁力链接不计入下载任务数，直接添加进下载列表
  // 磁力链接需设置解析完成回调
  BtMagnetTaskPtr magnet_task = std::static_pointer_cast<BtMagnetTask>(task);
  if (magnet_task) {
    magnet_task->SetParseCompletedCallback(
        base::BindOnce(&UserSelfMgr::OnParseComplete, shared_from_this()));
  }
  return task->Start();
}

DownloadErrorCode UserSelfMgr::ExecTorrent(TaskBasePtr task) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  TorrentTaskParamPtr torrent_param =
      std::static_pointer_cast<TorrentTaskParam>(
          task->GetTaskInfo()->GetTaskParam());
  if (IsCanStart(task)) {
    torrent_param->is_download_now = true;
    if (!SystemCloudConfig::GetInstance()->IsBtAutoDownload()) {
      LOG(INFO) << "task is not auto download! task_id = " << torrent_param->id;
      torrent_param->is_download_now = false;
      task->SetTaskStatus(TaskStatus::kPaused);
    } else {
      task->SetTaskStatus(TaskStatus::kDownloading);
    }
  } else {
    task->SetTaskStatus(TaskStatus::kWaitting);
  }

  err = task->Start();
  return err;
}

DownloadErrorCode UserSelfMgr::ExecNoneBt(TaskBasePtr task) {
  DownloadErrorCode err = DownloadErrorCode::kNormal;
  if (IsCanStart(task)) {
    err = task->Start();
  } else {
    LOG(WARNING) << "task is waitting";
  }
  return err;
}

DownloadErrorCode UserSelfMgr::CreateSavePath(const std::string& save_path) {
  DownloadErrorCode err_code = DownloadErrorCode::kNormal;
  base::FilePath file_path = nas::path_unit::BaseFilePathFromU8(save_path);
  if (!base::DirectoryExists(file_path)) {
    base::File::Error err;
    if (!base::CreateDirectoryAndGetError(file_path, &err)) {
      LOG(ERROR) << "create save path failed: "
                 << base::File::ErrorToString(err);
      err_code = DownloadErrorCode::kCreateDirFailed;
    }
  }
  return err_code;
}

DownloadErrorCode UserSelfMgr::IsProtectedPath(const std::string& save_path) {
  DownloadErrorCode err_code = DownloadErrorCode::kNormal;
  std::string convert_temp_path = nas::path_unit::UnifiedSlash(save_path);
  nas::path_unit::RemoveLastSlash(convert_temp_path);
  nas::path_unit::ProtectedPathCheck protected_path;
  if (protected_path.IsProtectedInPath(convert_temp_path)) {
    LOG(ERROR) << save_path << "is protected path, do not download";
    err_code = DownloadErrorCode::kProtectedPath;
  }
  return err_code;
}

DownloadErrorCode UserSelfMgr::HasWriteAccess(const std::string& save_path) {
  DownloadErrorCode err_code = DownloadErrorCode::kNormal;
  std::string guid_file = base::StringPrintf(
      "%s/%s.tmp", save_path.c_str(),
      base::GUID::GenerateRandomV4().AsLowercaseString().c_str());
  base::FilePath temp_path = nas::path_unit::BaseFilePathFromU8(guid_file);
  base::File temp_file(temp_path,
                       base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  if (!temp_file.IsValid()) {
    LOG(WARNING) << "save path is not write access! save path is " << save_path;
    err_code = DownloadErrorCode::kCreateFileFailed;
  }
  temp_file.Close();
  nas::utils::DeleteFile(guid_file);
  return err_code;
}

}  // namespace nas