/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-13 15:13:55
 */
#ifndef NAS_DOWNLOAD_SERVER_SRC_DB_DOWNLOAD_INFO_STORE_H_
#define NAS_DOWNLOAD_SERVER_SRC_DB_DOWNLOAD_INFO_STORE_H_

#include <map>
#include <memory>
#include <vector>

#include "base/files/file_path.h"
#include "base/task/sequenced_task_runner.h"

#include "nas/common/context_field.h"
#include "nas/common/db/sqlite_store_base.h"
#include "nas/download_service/download_server/src/task/task_common_define.h"
#include "nas/download_service/download_server/src/task/task_info.h"
#include "nas/download_service/download_server/src/user_info.h"

namespace nas {

class DownloadInfoStore : public SqliteStoreBase {
 public:
  DownloadInfoStore(
      const base::FilePath& path,
      scoped_refptr<base::SequencedTaskRunner> client_task_runner,
      scoped_refptr<base::SequencedTaskRunner> background_task_runner);

  DownloadInfoStore() = delete;
  DownloadInfoStore(const DownloadInfoStore&) = delete;
  DownloadInfoStore& operator=(const DownloadInfoStore&) = delete;

  void InitInBackground();

  std::vector<std::string> GetAllTableName() override;

 public:
  void AddTaskInfo(const std::string& user_id, const TaskInfoPtr task_info);
  void UpdateTaskInfo(const std::string& user_id, const TaskInfoPtr task_info);

  void DeleteTaskInfo(const std::string& user_id, const std::string& task_id);

  void GetTaskInfoList(const std::string& user_id,
                       std::vector<TaskInfoPtr>& task_info_list);

  void MakeSureUserInfoTableExist();
  void AddUserInfo(const std::string& user_id, const UserInfoPtr user_info);
  void UpdateUserInfo(const std::string& user_id, const UserInfoPtr user_info);
  void GetAllUsersInfo(std::map<std::string, UserInfoPtr>& user_info_list);

 private:
  void MakeSureTaskInfoTableExist(const std::string& user_id);
  bool CreateDatabaseSchema() override;

  // Initialize the data base.
  bool DoInitializeDatabase() override;

  // Commit our pending operations to   the database.
  void DoCommit() override{};
  void RecordOpenDBProblem() override{};

  void AddUser(const std::string& user_id);

 private:
  std::set<std::string> users_;
};

}  // namespace nas

#endif