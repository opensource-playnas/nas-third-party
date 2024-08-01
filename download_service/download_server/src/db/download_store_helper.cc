/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-13 20:21:21
 */

#include "download_store_helper.h"
#include "utils/utils.h"

namespace nas {

DownloadStoreHelper* DownloadStoreHelper::GetInstance() {
  return base::Singleton<DownloadStoreHelper>::get();
}

void DownloadStoreHelper::Init() {
  auto db_file = utils::GetDownloadConfigDir().Append(
      FILE_PATH_LITERAL("download_server.db"));

  db_cache_thread_ = std::make_unique<base::Thread>("cache_db_data");
  db_cache_thread_->Start();
  db_backend_thread_ = std::make_unique<base::Thread>("backend_db");
  db_backend_thread_->Start();

  download_info_store_ = base::MakeRefCounted<DownloadInfoStore>(
      db_file, db_cache_thread_->task_runner(),
      db_backend_thread_->task_runner());
}

void DownloadStoreHelper::UnInit() {
  if (download_info_store_) {
    download_info_store_->Close();
  }

  if (db_cache_thread_) {
    db_cache_thread_->Stop();
  }

  if (db_backend_thread_) {
    db_backend_thread_->Stop();
  }

  download_info_store_.reset();
}

void DownloadStoreHelper::Reset() {
  if (download_info_store_) {
    download_info_store_->ClearTableData(base::BindOnce([](bool ok) {
      if (!ok) {
        LOG(ERROR) << "clear download info failed ";
      } else {
        LOG(INFO) << "clear download info success";
      }
    }));
  }
}

void DownloadStoreHelper::AddTaskInfo(const std::string& user_id,
                                      const TaskInfoPtr task_info) {
  download_info_store_->AddTaskInfo(user_id, task_info);
}

void DownloadStoreHelper::UpdateTaskInfo(const std::string& user_id,
                                         const TaskInfoPtr task_info) {
  download_info_store_->UpdateTaskInfo(user_id, task_info);
}

void DownloadStoreHelper::DeleteTaskInfo(const std::string& user_id,
                                         const std::string& task_id) {
  download_info_store_->DeleteTaskInfo(user_id, task_id);
}

void DownloadStoreHelper::GetTaskInfoList(
    const std::string& user_id,
    std::vector<TaskInfoPtr>& task_info_list) {
  download_info_store_->GetTaskInfoList(user_id, task_info_list);
}

void DownloadStoreHelper::AddUserInfo(const std::string& user_id,
                                      const UserInfoPtr user_info) {
  download_info_store_->AddUserInfo(user_id, user_info);
}

void DownloadStoreHelper::UpdateUserInfo(const std::string& user_id,
                                         const UserInfoPtr user_info) {
  download_info_store_->UpdateUserInfo(user_id, user_info);
}

void DownloadStoreHelper::GetAllUsersInfo(
    std::map<std::string, UserInfoPtr>& user_info_list) {
  download_info_store_->GetAllUsersInfo(user_info_list);
}

DownloadStoreHelper::DownloadStoreHelper() {}

DownloadStoreHelper::~DownloadStoreHelper() {}

}  // namespace nas
