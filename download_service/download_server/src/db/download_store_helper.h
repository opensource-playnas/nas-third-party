/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-13 20:21:17
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_DB_DOWNLOAD_STORE_HELPER_H_
#define NAS_DOWNLOAD_SERVER_SRC_DB_DOWNLOAD_STORE_HELPER_H_

#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/threading/thread.h"
#include "download_info_store.h"
#include "nas/download_service/download_server/src/task/task_info.h"

namespace nas {
class DownloadStoreHelper {
 public:
  static DownloadStoreHelper* GetInstance();
  DownloadStoreHelper(const DownloadStoreHelper&) = delete;
  DownloadStoreHelper& operator=(const DownloadStoreHelper&) = delete;

  void Init();
  void UnInit();
  void Reset();

  void AddTaskInfo(const std::string& user_id, const TaskInfoPtr task_info);
  void UpdateTaskInfo(const std::string& user_id, const TaskInfoPtr task_info);
  void DeleteTaskInfo(const std::string& user_id, const std::string& task_id);
  void GetTaskInfoList(const std::string& user_id,
                       std::vector<TaskInfoPtr>& task_info_list);
  void AddUserInfo(const std::string& user_id, const UserInfoPtr user_info);
  void UpdateUserInfo(const std::string& user_id, const UserInfoPtr user_info);
  void GetAllUsersInfo(std::map<std::string, UserInfoPtr>& user_info_list);

 private:
  // This object is a singleton:
  DownloadStoreHelper();
  ~DownloadStoreHelper();
  friend struct base::DefaultSingletonTraits<DownloadStoreHelper>;

 private:
  std::unique_ptr<base::Thread> db_cache_thread_;
  std::unique_ptr<base::Thread> db_backend_thread_;
  scoped_refptr<DownloadInfoStore> download_info_store_;

};  // NAS_DOWNLOAD_SERVER_SRC_DB_DOWNLOAD_STORE_HELPER_H_
}  // namespace nas

#endif