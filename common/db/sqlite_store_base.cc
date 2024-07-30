/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-12-02 16:30:10
 */

#include "nas/common/db/sqlite_store_base.h"

#include <utility>

#include "base/bind.h"
#include "base/check.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_executor.h"
#include "base/time/time.h"

#include "sql/database.h"
#include "sql/error_delegate_util.h"
#include "sqlite_store_base.h"

namespace nas {

typedef base::OnceCallback<void(int)> ExecuteCallback;

SqliteStoreBase::SqliteStoreBase(
    const base::FilePath& path,
    scoped_refptr<base::SequencedTaskRunner> client_task_runner,
    scoped_refptr<base::SequencedTaskRunner> background_task_runner)
    : path_(path),
      background_task_runner_(background_task_runner),
      client_task_runner_(client_task_runner) {}

SqliteStoreBase::~SqliteStoreBase() {
  LOG(ERROR) << __func__ ;

  // 析构前必须关闭DB
  CHECK(!db_.get()) << "Close should already have been called.";
}

void SqliteStoreBase::Close() {
  LOG(INFO) << __func__;

  if (background_task_runner_->RunsTasksInCurrentSequence()) {
    DoCloseInBackground();
  } else {
    // Must close on the background runner.
    PostBackgroundTask(FROM_HERE,
                       base::BindOnce(&SqliteStoreBase::DoCloseInBackground,
                                      base::Unretained(this)));
  }
}

void SqliteStoreBase::Close(CloseCallback callback) {
  if (background_task_runner_->RunsTasksInCurrentSequence()) {
    DoCloseInBackground();
    PostClientTask(FROM_HERE, base::BindOnce(std::move(callback)));
  } else {
    // Must close on the background runner.
    PostBackgroundTask(
        FROM_HERE, base::BindOnce(
                       [](SqliteStoreBase* self, CloseCallback callback) {
                         self->DoCloseInBackground();
                         self->PostClientTask(
                             FROM_HERE, base::BindOnce(std::move(callback)));
                       },
                       base::Unretained(this), std::move(callback)));
  }
}

void SqliteStoreBase::SetBeforeCommitCallback(base::RepeatingClosure callback) {
  base::AutoLock locked(before_commit_callback_lock_);
  before_commit_callback_ = std::move(callback);
}

void SqliteStoreBase::ClearTableData(
    ExecuteClearDataCallback execute_callback) {
  LOG(INFO) << __func__;

  PostBackgroundTask(
      FROM_HERE,
      base::BindOnce(
          [](SqliteStoreBase* store,
             ExecuteClearDataCallback execute_callback) {
            bool ok = false;
            do {
              if (!store->db()) {
                LOG(ERROR) << "add db ,db is nullptr";
                break;
              }

              std::vector<std::string> table_names = store->GetAllTableName();
              bool is_failed = false;
              for (const auto& name : table_names) {
                std::string delete_stmt(
                    base::StringPrintf("DELETE FROM '%s'", name.c_str()));

                if (!store->db()->Execute(delete_stmt.c_str())) {
                  LOG(ERROR) << "delete table failed, table name: " << name;
                  is_failed = true;
                  continue;
                }
              }

              ok = is_failed ? false : true;
              LOG(INFO) << "delete tables " << (ok ? "success" : "failed");
              store->PostClientTask(
                  FROM_HERE, base::BindOnce(std::move(execute_callback), ok));
            } while (false);
          },
          base::Unretained(this), std::move(execute_callback)));
}

bool SqliteStoreBase::IsInializeDataBase() {
  return initialized_;
}

bool SqliteStoreBase::InitializeDatabase() {
  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  if (initialized_ || corruption_detected_) {
    // Return false if we were previously initialized but the DB has since been
    // closed, or if corruption caused a database reset during initialization.
    return db_ != nullptr;
  }

  const base::FilePath dir = path_.DirName();
  if (!base::PathExists(dir) && !base::CreateDirectory(dir)) {
    LOG(INFO) << __func__ << ", create dir failed, dir: " << dir;
    RecordPathDoesNotExistProblem();
    return false;
  }

  db_ = std::make_unique<sql::Database>();
  // db_->set_histogram_tag(histogram_tag_);

  // base::Unretained is safe because |this| owns (and therefore outlives) the
  // sql::Database held by |db_|.
  db_->set_error_callback(base::BindRepeating(
      &SqliteStoreBase::DatabaseErrorCallback, base::Unretained(this)));

  bool new_db = !base::PathExists(path_);

  if (!db_->Open(path_)) {
    LOG(INFO) << __func__ << ", Unable to open " << path_ << " DB.";
    RecordOpenDBProblem();
    Reset();
    return false;
  }
  db_->Preload();

  if (!CreateDatabaseSchema()) {
    LOG(INFO) << __func__ << ", Unable to initialize " << path_
              << " DB tables.";
    Reset();
    return false;
  }

  initialized_ = DoInitializeDatabase();
  if (!initialized_) {
    LOG(INFO) << __func__ << ", Unable to initialize " << path_ << " DB.";

    RecordOpenDBProblem();
    Reset();
    return false;
  }

  if (new_db) {
    RecordNewDBFile();
  } else {
    RecordDBLoaded();
  }

  return true;
}

bool SqliteStoreBase::DoInitializeDatabase() {
  return true;
}

void SqliteStoreBase::Reset() {
  LOG(INFO) << __func__;
  
  if (db_ && db_->is_open())
    db_->Raze();
  meta_table_.Reset();
  db_.reset();
}

void SqliteStoreBase::Commit() {
  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  {
    base::AutoLock locked(before_commit_callback_lock_);
    if (!before_commit_callback_.is_null())
      before_commit_callback_.Run();
  }

  DoCommit();
}

void SqliteStoreBase::PostBackgroundTask(const base::Location& origin,
                                         base::OnceClosure task) {
  if (!background_task_runner_->PostTask(origin, std::move(task))) {
    LOG(WARNING) << "Failed to post task from " << origin.ToString()
                 << " to background_task_runner_.";
  }
}

void SqliteStoreBase::PostClientTask(const base::Location& origin,
                                     base::OnceClosure task) {
  if (!client_task_runner_->PostTask(origin, std::move(task))) {
    LOG(WARNING) << "Failed to post task from " << origin.ToString()
                 << " to client_task_runner_.";
  }
}

int SqliteStoreBase::SyncWaitBackgroundTaskCompleted(
    const base::Location& origin,
    ReturnErrnoCallback task) {
  base::SingleThreadTaskExecutor task_executor(base::MessagePumpType::DEFAULT);

  base::RunLoop loop;
  int err_noum = 0;

  ExecuteCallback callback = base::BindOnce(
      [](base::RunLoop* loop, int& out_errno, int err) {
        out_errno = err;
        loop->Quit();
      },
      &loop, std::ref(err_noum));

  if (!background_task_runner_->PostTaskAndReplyWithResult(
          origin, std::move(task), std::move(callback))) {
    LOG(WARNING) << "Failed to post task from " << origin.ToString()
                 << " to background_task_runner_.";
  }

  loop.Run();

  return err_noum;
}

void SqliteStoreBase::FlushAndNotifyInBackground(base::OnceClosure callback) {
  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  Commit();
  if (callback)
    PostClientTask(FROM_HERE, std::move(callback));
}

void SqliteStoreBase::DoCloseInBackground() {
  LOG(INFO) << __func__;

  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());
  // Commit any pending operations
  Commit();

  meta_table_.Reset();
  db_.reset();
}

void SqliteStoreBase::DatabaseErrorCallback(int error, sql::Statement* stmt) {
  LOG(ERROR) << __func__ << " error: " << error;
  // 屏蔽掉出错就重置数据库
  return;


  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  if (!sql::IsErrorCatastrophic(error))
    return;

  // TODO(shess): Running KillDatabase() multiple times should be
  // safe.
  if (corruption_detected_)
    return;

  corruption_detected_ = true;

  // Don't just do the close/delete here, as we are being called by |db| and
  // that seems dangerous.
  // TODO(shess): Consider just calling RazeAndClose() immediately.
  // db_ may not be safe to reset at this point, but RazeAndClose()
  // would cause the stack to unwind safely with errors.
  PostBackgroundTask(FROM_HERE, base::BindOnce(&SqliteStoreBase::KillDatabase,
                                               base::Unretained(this)));
}

void SqliteStoreBase::KillDatabase() {
  LOG(ERROR) << __func__ ;

  DCHECK(background_task_runner_->RunsTasksInCurrentSequence());

  if (db_) {
    // This Backend will now be in-memory only. In a future run we will recreate
    // the database. Hopefully things go better then!
    db_->RazeAndClose();
    meta_table_.Reset();
    db_.reset();
  }
}
}  // namespace nas
