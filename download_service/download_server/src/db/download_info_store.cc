/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-13 15:15:25
 */
#include "download_info_store.h"

#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/time/time.h"

#include "sql/meta_table.h"
#include "sql/statement.h"
#include "sql/transaction.h"

#include "nas/common/path/file_path_unit.h"

namespace nas {
const static char* kUserInfoTableName = "user_info";
const static char* kTaskInfo = "_task_info";
bool GetTaskInfoListFromSQLStatement(sql::Statement& smt,
                                     std::vector<TaskInfoPtr>& task_info_list);
typedef base::OnceCallback<void(bool)> ExecuteCallback;

DownloadInfoStore::DownloadInfoStore(
    const base::FilePath& path,
    scoped_refptr<base::SequencedTaskRunner> client_task_runner,
    scoped_refptr<base::SequencedTaskRunner> background_task_runner)
    : SqliteStoreBase(path, client_task_runner, background_task_runner) {
  PostBackgroundTask(FROM_HERE,
                     base::BindOnce(&DownloadInfoStore::InitInBackground,
                                    base::Unretained(this)));
}

void DownloadInfoStore::InitInBackground() {
  DCHECK(background_task_runner()->RunsTasksInCurrentSequence());
  if (!InitializeDatabase()) {
    LOG(ERROR) << "init database failed!";
  } else {
    LOG(INFO) << "init db success";
  }
}

std::vector<std::string> DownloadInfoStore::GetAllTableName() {
  std::vector<std::string> table_names;
  for (const auto& user : users_) {
    table_names.push_back(user + kTaskInfo);
  }
  table_names.push_back(kUserInfoTableName);
  return table_names;
}

void DownloadInfoStore::MakeSureTaskInfoTableExist(const std::string& user_id) {
  PostBackgroundTask(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store, std::string user_id) {
            int errno_code = 1;
            do {
              if (!store->db()) {
                LOG(ERROR) << " db is nullptr";
                errno_code = 0;
                break;
              }

              std::string user_task_progress_info_table = user_id + kTaskInfo;

              if (store->db()->DoesTableExist(user_task_progress_info_table)) {
                break;
              }

              std::string sql = base::StringPrintf(
                  "CREATE TABLE '%s' ("
                  "  id TEXT NOT NULL PRIMARY KEY,"
                  "  source TEXT NOT NULL,"
                  "  save_path TEXT NOT NULL,"
                  "  download_type INTEGER,"
                  "  file_name TEXT NOT NULL,"
                  "  file_size INTEGER,"
                  "  total_progress REAL,"
                  "  download_speed REAL,"
                  "  upload_speed REAL,"
                  "  task_status INTEGER,"
                  "  create_time INTEGER,"
                  "  finish_time INTEGER,"
                  "  delete_time INTEGER,"
                  "  err_code INTEGER,"
                  "  is_started INTEGER)",
                  user_task_progress_info_table.c_str());

              errno_code = store->db()->Execute(sql.c_str());

              LOG(INFO) << __func__ << ": errno_code = " << errno_code;
            } while (false);
          },
          base::Unretained(this), user_id));
}

void DownloadInfoStore::AddTaskInfo(const std::string& user_id,
                                    const TaskInfoPtr task_info) {
  MakeSureTaskInfoTableExist(user_id);

  PostBackgroundTask(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store, std::string user_id,
             const TaskInfoPtr task_info) {
            do {
              std::string user_task_progress_info_table = user_id + kTaskInfo;
              if (!store->db()->DoesTableExist(user_task_progress_info_table)) {
                LOG(ERROR) << "user_task_progress_info_table is not exist!";
                break;
              }

              std::string sql = base::StringPrintf(
                  "INSERT INTO '%s' (id, source, "
                  "save_path, download_type, file_name, file_size, "
                  "total_progress, download_speed, upload_speed, "
                  "task_status, create_time, finish_time, delete_time, "
                  "err_code, is_started) "
                  "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                  user_task_progress_info_table.c_str());

              sql::Statement add_statement(
                  store->db()->GetUniqueStatement(sql.c_str()));
              if (!add_statement.is_valid()) {
                LOG(ERROR) << "add db ,add_statement not is_valid";
                break;
              }
              int column = 0;
              TaskParamPtr task_param = task_info->GetTaskParam();
              TaskBaseInfoPtr task_base_info = task_info->GetTaskBaseInfo();
              TaskProgressInfoPtr progress_info = task_info->GetProgressInfo();
              TaskTimeStampPtr task_time_stamp = task_info->GetTaskTimeStamp();
              FileAttribute file_attribute = task_base_info->GetTaskAttribute();

              add_statement.BindString(column++, task_param->id);
              add_statement.BindString(column++, task_param->url);
              add_statement.BindString(column++, task_param->save_path);
              add_statement.BindInt(column++, task_param->type);
              add_statement.BindString(column++, file_attribute.file_name);
              add_statement.BindInt64(column++, file_attribute.file_size);
              add_statement.BindInt64(column++,
                                      progress_info->GetTotalProgress());
              add_statement.BindInt64(column++,
                                      progress_info->GetDownloadSpeed());
              add_statement.BindInt64(column++,
                                      progress_info->GetUploadSpeed());
              add_statement.BindInt(column++, task_info->GetTaskStatus());
              add_statement.BindInt64(column++, task_time_stamp->create_time);
              add_statement.BindInt64(column++, task_time_stamp->finish_time);
              add_statement.BindInt64(column++, task_time_stamp->delete_time);
              add_statement.BindInt(column++, progress_info->GetErrorCode());
              add_statement.BindBool(column++, task_info->IsStarted());

              if (!add_statement.Run()) {
                LOG(ERROR) << "Could not add task_info to the DB.";
                break;
              }

              LOG(INFO) << "add task_id = " << task_param->id << " success!";

            } while (false);
          },
          base::Unretained(this), user_id,
          std::make_shared<TaskInfo>(*task_info)));
}

void DownloadInfoStore::UpdateTaskInfo(const std::string& user_id,
                                       const TaskInfoPtr task_info) {
  PostBackgroundTask(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store, const std::string& user_id,
             const TaskInfoPtr task_info) {
            do {
              std::string user_task_progress_info_table = user_id + kTaskInfo;

              if (!store->db()) {
                LOG(ERROR) << " db is nullptr";
                break;
              }

              if (!store->db()->DoesTableExist(user_task_progress_info_table)) {
                break;
              }

              TaskParamPtr task_param = task_info->GetTaskParam();
              TaskBaseInfoPtr task_base_info = task_info->GetTaskBaseInfo();
              TaskProgressInfoPtr progress_info = task_info->GetProgressInfo();
              TaskTimeStampPtr task_time_stamp = task_info->GetTaskTimeStamp();
              FileAttribute file_attribute = task_base_info->GetTaskAttribute();
#if BUILDFLAG(IS_WIN)
              std::string sql = base::StringPrintf(
                  "UPDATE '%s' SET file_name='%s', "
                  "file_size='%lld', total_progress='%f', "
                  "download_speed='%f', upload_speed='%f', "
                  "task_status='%d', create_time='%lld', finish_time='%lld', "
                  "delete_time='%lld', err_code='%d', is_started='%d' "
                  "WHERE id='%s'",
                  user_task_progress_info_table.c_str(),
                  file_attribute.file_name.c_str(), file_attribute.file_size,
                  progress_info->GetTotalProgress(),
                  progress_info->GetDownloadSpeed(),
                  progress_info->GetUploadSpeed(), task_info->GetTaskStatus(),
                  task_time_stamp->create_time, task_time_stamp->finish_time,
                  task_time_stamp->delete_time, progress_info->GetErrorCode(),
                  task_info->IsStarted(), task_param->id.c_str());
#else
              std::string sql = base::StringPrintf(
                  "UPDATE '%s' SET file_name='%s', "
                  "file_size='%ld', total_progress='%f', "
                  "download_speed='%f', upload_speed='%f', "
                  "task_status='%d', create_time='%ld', finish_time='%ld', "
                  "delete_time='%ld', err_code='%d', is_started='%d' "
                  "WHERE id='%s'",
                  user_task_progress_info_table.c_str(),
                  file_attribute.file_name.c_str(), file_attribute.file_size,
                  progress_info->GetTotalProgress(),
                  progress_info->GetDownloadSpeed(),
                  progress_info->GetUploadSpeed(), task_info->GetTaskStatus(),
                  task_time_stamp->create_time, task_time_stamp->finish_time,
                  task_time_stamp->delete_time, progress_info->GetErrorCode(),
                  task_info->IsStarted(), task_param->id.c_str());
#endif


              if (!store->db()->Execute(sql.c_str())) {
                LOG(ERROR) << "update task_info failed, task_id = "
                           << task_param->id;
                break;
              }

            } while (false);
          },
          base::Unretained(this), user_id,
          std::make_shared<TaskInfo>(*task_info)));
}

void DownloadInfoStore::DeleteTaskInfo(const std::string& user_id,
                                       const std::string& task_id) {
  PostBackgroundTask(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store, std::string user_id,
             std::string task_id) {
            do {
              std::string user_task_progress_info_table = user_id + kTaskInfo;

              if (!store->db()->DoesTableExist(user_task_progress_info_table)) {
                break;
              }

              std::string sql = base::StringPrintf(
                  "DELETE FROM '%s' WHERE id='%s'",
                  user_task_progress_info_table.c_str(), task_id.c_str());

              if (!store->db()->Execute(sql.c_str())) {
                LOG(ERROR) << "delete task_info failed, task_id = " << task_id;
                break;
              }

              LOG(INFO) << "delete task_id = " << task_id << " success!";
            } while (false);
          },
          base::Unretained(this), user_id, task_id));
}

void DownloadInfoStore::GetTaskInfoList(
    const std::string& user_id,
    std::vector<TaskInfoPtr>& task_info_list) {
  SyncWaitBackgroundTaskCompleted(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store, const std::string& user_id,
             std::vector<TaskInfoPtr>& task_info_list) -> int {
            do {
              if (!store->db()) {
                LOG(ERROR) << " db is nullptr";
                break;
              }

              std::string user_task_progress_info_table = user_id + kTaskInfo;

              if (!store->db()->DoesTableExist(user_task_progress_info_table)) {
                break;
              }

              sql::Statement smt(store->db()->GetUniqueStatement(
                  base::StringPrintf("SELECT * FROM '%s'",
                                     user_task_progress_info_table.c_str())
                      .c_str()));

              GetTaskInfoListFromSQLStatement(smt, task_info_list);
            } while (false);
            return 0;
          },
          base::Unretained(this), user_id, std::ref(task_info_list)));
}

void DownloadInfoStore::MakeSureUserInfoTableExist() {
  PostBackgroundTask(
      FROM_HERE, base::BindOnce(
                     [](DownloadInfoStore* store) {
                       int errno_code = 1;
                       do {
                         if (!store->db()) {
                           LOG(ERROR) << " db is nullptr";
                           errno_code = 0;
                           break;
                         }

                         if (store->db()->DoesTableExist(kUserInfoTableName)) {
                           break;
                         }

                         std::string sql = base::StringPrintf(
                             "CREATE TABLE '%s' ("
                             "  user_id TEXT NOT NULL PRIMARY KEY,"
                             "  accumulate_download INTEGER NOT NULL,"
                             "  accumulate_upload INTEGER NOT NULL)",
                             kUserInfoTableName);

                         errno_code = store->db()->Execute(sql.c_str());

                         LOG(INFO) << __func__ << ": errno_code = " << errno_code;
                       } while (false);
                     },
                     base::Unretained(this)));
}

void DownloadInfoStore::AddUserInfo(const std::string& user_id,
                                    const UserInfoPtr user_info) {
  MakeSureUserInfoTableExist();

  PostBackgroundTask(
      FROM_HERE, base::BindOnce(
                     [](DownloadInfoStore* store, const std::string& user_id,
                        const UserInfoPtr user_info) {
                       do {
                         if (!store->db()) {
                           LOG(ERROR) << " db is nullptr";
                           break;
                         }

                         std::string sql = base::StringPrintf(
                             "INSERT INTO '%s' (user_id, accumulate_download, "
                             "accumulate_upload) "
                             "VALUES (?,?,?)",
                             kUserInfoTableName);

                         sql::Statement add_statement(
                             store->db()->GetUniqueStatement(sql.c_str()));
                         if (!add_statement.is_valid()) {
                           LOG(ERROR) << "add db ,add_statement not is_valid";

                           break;
                         }
                         DownloadStatisticsInfo statistics_info =
                             user_info->GetDownloadStatisticsInfo();
                         int column = 0;
                         add_statement.BindString(column++, user_id);
                         add_statement.BindInt64(
                             column++, statistics_info.accumulate_download);
                         add_statement.BindInt64(
                             column++, statistics_info.accumulate_upload);

                         if (!add_statement.Run()) {
                           LOG(ERROR) << "Could not add user_info to the DB.";
                           break;
                         }
                         store->AddUser(user_id);
                         LOG(INFO) << "add user_id = " << user_id
                                   << " statistics info success!";

                       } while (false);
                     },
                     base::Unretained(this), user_id,
                     std::make_shared<UserInfo>(*user_info)));
}

void DownloadInfoStore::UpdateUserInfo(const std::string& user_id,
                                       const UserInfoPtr user_info) {
  PostBackgroundTask(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store, const std::string& user_id,
             const UserInfoPtr user_info) {
            do {
              DownloadStatisticsInfo statistics_info =
                  user_info->GetDownloadStatisticsInfo();

              if (!store->db()) {
                LOG(ERROR) << " db is nullptr";
                break;
              }

              std::string sql = base::StringPrintf(
                  "UPDATE '%s' SET accumulate_download='%llu', "
                  "accumulate_download='%llu' "
                  "WHERE user_id='%s'",
                  kUserInfoTableName, statistics_info.accumulate_download,
                  statistics_info.accumulate_upload, user_id.c_str());

              if (!store->db()->Execute(sql.c_str())) {
                LOG(ERROR) << "update statistics_info failed, user_id = "
                           << user_id;
                break;
              }

            } while (false);
          },
          base::Unretained(this), user_id,
          std::make_shared<UserInfo>(*user_info)));
}

void DownloadInfoStore::GetAllUsersInfo(
    std::map<std::string, UserInfoPtr>& user_info_list) {
  SyncWaitBackgroundTaskCompleted(
      FROM_HERE,
      base::BindOnce(
          [](DownloadInfoStore* store,
             std::map<std::string, UserInfoPtr>& user_info_list) -> int {
            do {
              if (!store->db()) {
                LOG(ERROR) << " db is nullptr";
                break;
              }

              if (!store->db()->DoesTableExist(kUserInfoTableName)) {
                break;
              }

              std::string sql =
                  base::StringPrintf("SELECT * FROM '%s'", kUserInfoTableName);

              sql::Statement stmt(store->db()->GetUniqueStatement(sql.c_str()));
              while (stmt.Step()) {
                int column = 0;
                std::string user_id = stmt.ColumnString(column++);
                UserInfoPtr info = std::make_shared<UserInfo>(user_id);
                DownloadStatisticsInfo statistics_info;

                statistics_info.accumulate_download =
                    stmt.ColumnInt64(column++);
                statistics_info.accumulate_upload = stmt.ColumnInt64(column++);
                info->SetDownloadStatisticsInfo(statistics_info);
                user_info_list[user_id] = info;
                store->AddUser(user_id);
              }

            } while (false);
            return 0;
          },
          base::Unretained(this), std::ref(user_info_list)));
}

bool GetTaskInfoListFromSQLStatement(sql::Statement& stmt,
                                     std::vector<TaskInfoPtr>& task_info_list) {
  bool ok = true;
  while (stmt.Step()) {
    int column = 0;
    std::string id = stmt.ColumnString(column++);
    std::string url = stmt.ColumnString(column++);
    std::string save_path = stmt.ColumnString(column++);
    DownloadType type = DownloadType(stmt.ColumnInt(column++));
    std::string file_name = stmt.ColumnString(column++);

    TaskInfoPtr task_info = std::make_shared<TaskInfo>();
    TaskParamPtr task_param = nullptr;
    if (type == DownloadType::kMagnet || type == DownloadType::kTorrent) {
      task_param = std::make_shared<TorrentTaskParam>();
    } else {
      task_param = std::make_shared<TaskParam>();
    }
    TaskBaseInfoPtr task_base_info = task_info->GetTaskBaseInfo();
    TaskProgressInfoPtr progress_info = task_info->GetProgressInfo();
    TaskTimeStampPtr task_time_stamp = task_info->GetTaskTimeStamp();

    if (!task_info || !task_param || !task_base_info || !progress_info ||
        !task_time_stamp) {
      continue;
    }

    task_param->id = id;
    task_param->url = url;
    task_param->save_path = save_path;
    task_param->type = type;
    task_param->file_name = file_name;
    task_param->is_from_db_resume = true;
    task_info->SetTaskParam(task_param);
    task_base_info->SetTaskName(task_param->file_name);
    task_base_info->SetTaskLandFileName(task_param->file_name);
    nas::path_unit::FileType file_type;
    if (type == DownloadType::kTorrent || type == DownloadType::kMagnet) {
      file_type = nas::path_unit::FileType::kBt;
    } else {
      file_type = nas::path_unit::GetFileType(task_param->file_name);
    }
    task_base_info->SetTaskFileType(std::to_string(file_type));
    task_base_info->SetTaskFileSize(stmt.ColumnInt64(column++));
    progress_info->SetTotalProgress(stmt.ColumnInt64(column++));
    progress_info->SetDownloadSpeed(stmt.ColumnInt64(column++));
    progress_info->SetUploadSpeed(stmt.ColumnInt64(column++));
    nas::TaskStatus status = nas::TaskStatus(stmt.ColumnInt(column++));
    task_info->SetTaskStatus(status);
    task_time_stamp->create_time = stmt.ColumnInt64(column++);
    task_time_stamp->finish_time = stmt.ColumnInt64(column++);
    task_time_stamp->delete_time = stmt.ColumnInt64(column++);
    progress_info->SetErrorCode(DownloadErrorCode(stmt.ColumnInt(column++)));
    task_info->SetIsStarted(stmt.ColumnInt(column++));
    task_info_list.push_back(task_info);
  }

  return ok;
}

bool DownloadInfoStore::CreateDatabaseSchema() {
  DCHECK(db());
  return true;
}

bool DownloadInfoStore::DoInitializeDatabase() {
  return true;
}

void DownloadInfoStore::AddUser(const std::string& user_id) {
  users_.insert(user_id);
}
}  // namespace nas
