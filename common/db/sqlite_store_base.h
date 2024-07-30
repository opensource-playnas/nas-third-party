/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-12-02 16:30:10
 */

#ifndef NAS_COMMON_SQLITE_STORE_BASE_H_
#define NAS_COMMON_SQLITE_STORE_BASE_H_
#include <memory>
#include <string>

#include "base/callback.h"
#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/thread_annotations.h"
#include "sql/database.h"
#include "sql/meta_table.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

/*
 *  封装一个基类,目的是对sqlite3数据库的操作都在一个线程内完成
 *
 */
namespace nas {
using ReturnErrnoCallback = base::OnceCallback<int()>;
using ExecuteClearDataCallback = base::OnceCallback<void(bool)>;

class SqliteStoreBase : public base::RefCountedThreadSafe<SqliteStoreBase> {
 public:
  SqliteStoreBase(const SqliteStoreBase&) = delete;
  SqliteStoreBase& operator=(const SqliteStoreBase&) = delete;

  void Flush(base::OnceClosure callback);

  // 提交所有操作并关闭数据库
  void Close();
  // callback 在 client task runner 中调用
  using CloseCallback = base::OnceCallback<void()>;
  void Close(CloseCallback callback);

  void SetBeforeCommitCallback(base::RepeatingClosure callback);

  // 清空表数据
  void ClearTableData(ExecuteClearDataCallback execute_callback);

  // 获取所有表名称
  virtual std::vector<std::string> GetAllTableName() = 0;

 protected:
  friend class base::RefCountedThreadSafe<SqliteStoreBase>;

  SqliteStoreBase(
      const base::FilePath& path,
      scoped_refptr<base::SequencedTaskRunner> background_task_runner,
      scoped_refptr<base::SequencedTaskRunner> client_task_runner);

  virtual ~SqliteStoreBase();

  bool IsInializeDataBase();
  // 初始化数据库,在background thread中调用,调用其他子类方法前,确保已经被执行
  bool InitializeDatabase();

  virtual void RecordPathDoesNotExistProblem(){};
  virtual void RecordOpenDBProblem(){};
  virtual void RecordDBMigrationProblem(){};
  virtual void RecordNewDBFile(){};
  virtual void RecordDBLoaded(){};
  // 创建数据库
  virtual bool CreateDatabaseSchema() = 0;

  virtual bool DoInitializeDatabase();

  // Raze and reset the metatable and database, e.g. if errors are encountered
  // in initialization.
  void Reset();
  void Commit();
  virtual void DoCommit() = 0;

  // 投递一个任务到后台线程.
  void PostBackgroundTask(const base::Location& origin, base::OnceClosure task);
  // 投递一个任务到前台线程..
  void PostClientTask(const base::Location& origin, base::OnceClosure task);

  // 同步等待任务执行完成
  // return: errno
  int SyncWaitBackgroundTaskCompleted(const base::Location& origin,
                                      ReturnErrnoCallback task);

  sql::Database* db() { return db_.get(); }
  sql::MetaTable* meta_table() { return &meta_table_; }

  base::SequencedTaskRunner* background_task_runner() {
    return background_task_runner_.get();
  }
  base::SequencedTaskRunner* client_task_runner() {
    return client_task_runner_.get();
  }

 private:
  // Flushes (commits pending operations) on the background runner, and invokes
  // |callback| on the client thread when done.
  void FlushAndNotifyInBackground(base::OnceClosure callback);

  // Close the database on the background runner.
  void DoCloseInBackground();

  // Error-handling callback. On errors, the error number (and statement, if
  // available) will be passed to the callback.
  // Sets |corruption_detected_| and posts a task to the background runner to
  // kill the database.
  void DatabaseErrorCallback(int error, sql::Statement* stmt);

  // Kills the database in the case of a catastropic error.
  void KillDatabase();

  // 数据库文件路径
  const base::FilePath path_;

  std::unique_ptr<sql::Database> db_;
  sql::MetaTable meta_table_;

  bool initialized_ = false;
  bool corruption_detected_ = false;
  const scoped_refptr<base::SequencedTaskRunner> background_task_runner_;
  const scoped_refptr<base::SequencedTaskRunner> client_task_runner_;

  // Callback to be run before Commit.
  base::RepeatingClosure before_commit_callback_
      GUARDED_BY(before_commit_callback_lock_);
  // Guards |before_commit_callback_|.
  base::Lock before_commit_callback_lock_;
};  // class SqliteStoreBase
}  // namespace nas

#endif  // NAS_COMMON_SQLITE_STORE_BASE_H_
