/*
 * @Description: user self manager
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-06 10:54:33
 */
#ifndef NAS_DOWNLOAD_SERVER_SRC_USER_SELF_MGR_H_
#define NAS_DOWNLOAD_SERVER_SRC_USER_SELF_MGR_H_

#include "base/threading/thread.h"
#include "nas/common/context_field.h"
#include "nas/common/nas_thread.h"
#include "task/task_base.h"
#include "task/task_common_define.h"
#include "user_info.h"

namespace nas {
static const char* kUserThreadName = "user_thread";

using GetTaskListCallback =
    base::OnceCallback<void(const std::vector<TaskInfoPtr>& task_info_list)>;

typedef bool (*PfnOrder)(const TaskBasePtr& a, const TaskBasePtr& b);

class TaskComparator {
 public:
  void SetThreshold(int64_t threshold) { threshold_ = threshold; }

  bool operator()(const TaskBasePtr& left, const TaskBasePtr& right) {
    int64_t left_size =
        left->GetTaskInfo()->GetTaskBaseInfo()->GetTaskAttribute().file_size;
    int64_t right_size =
        right->GetTaskInfo()->GetTaskBaseInfo()->GetTaskAttribute().file_size;

    if (left_size < threshold_ && right_size < threshold_) {
      return left_size < right_size;
    } else if (left_size >= threshold_ && right_size >= threshold_) {
      return left->GetTaskInfo()->GetTaskTimeStamp()->create_time <
             right->GetTaskInfo()->GetTaskTimeStamp()->create_time;
    } else {
      return left_size < right_size;
    }
  }

 private:
  int64_t threshold_ = 0;
};

class UserSelfMgr : public std::enable_shared_from_this<UserSelfMgr> {
 public:
  struct TaskErrCode {
    std::string task_id;
    DownloadErrorCode err;
  };

  explicit UserSelfMgr(const ContextField& context_field,
                       scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  ~UserSelfMgr();

  void Stop();
  void Reset(base::WaitableEvent& wait_event);

 public:
  void CreateLinkTask(
      const download::v1::CreateLinkDownloadTaskRequest* request,
      const ContextField& context_field);

  void CreateTorrentTask(
      const download::v1::CreateTorrentDownloadTaskRequest* request,
      const ContextField& context_field);

  void Pause(const download::v1::PauseRequest* request,
             const ContextField& context_field);

  void PauseAll(const ContextField& context_field);

  void Resume(const download::v1::ResumeRequest* request,
              const ContextField& context_field);

  void ResumeAll(const ContextField& context_field);

  void Delete(const download::v1::DeleteRequest* request,
              const ContextField& context_field);

  void DeleteCategory(const download::v1::DeleteCategoryRequest* request,
                      const ContextField& context_field);

  void Retry(const download::v1::RetryRequest* request,
             const ContextField& context_field);

  void RetryAll(const ContextField& context_field);

  void Clean(const download::v1::CleanRequest* request,
             const ContextField& context_field);

  void CleanCategory(const download::v1::CleanCategoryRequest* request,
                     const ContextField& context_field);

  void Restore(const download::v1::RestoreRequest* request,
               const ContextField& context_field);

  void RestoreAll(const ContextField& context_field);

  void GetTaskInfoList(const ContextField& context_field,
                       GetTaskListCallback callback);

 public:
  void UpdateUserInfo(const ContextField& context_field);
  void SetUserInfo(const UserInfoPtr user_info);
  void TimerChcek();
  void ResumeFromDb(const std::vector<TaskInfoPtr>& task_info_list,
                    const ContextField& context_field);
  void StartTimer();
  void StopTimer();

 private:
  void ExecCreate(const TaskParamPtr task_param,
                  const ContextField& context_field,
                  std::vector<TaskBasePtr>* need_parse_task_list,
                  base::Value::List* success_list,
                  base::Value::List* failed_list);

  DownloadErrorCode CreateTask(const TaskParamPtr task_param,
                               const ContextField& context_field,
                               TaskBasePtr& task,
                               std::string& same_task_id);
  bool AddTask(const std::string& task_id, TaskBasePtr task);
  TaskBasePtr FindTask(const std::string& task_id);

  void AssembleMessage(const TaskOperation& operation,
                       const ContextField& context_field,
                       base::Value::List success_list,
                       base::Value::List failed_list,
                       base::Value::Dict* message_data,
                       DownloadErrorCode error = DownloadErrorCode::kNormal);

  std::string GetAction(const TaskOperation& operation);
  void ParseTasks(const std::vector<TaskBasePtr>& tasks);
  void BuildCreateMessage(DownloadErrorCode error,
                          const std::string& task_id,
                          const std::string& same_task_id,
                          base::Value::List* success_list,
                          base::Value::List* failed_list);

  DownloadErrorCode ExecPauseOperate(const std::string& task_id);
  DownloadErrorCode ExecResumeOperate(const std::string& task_id);
  DownloadErrorCode ExecDeleteOperate(const std::string& task_id,
                                      bool is_delete_local_file,
                                      bool is_judge_task_valid,
                                      base::Value::List* success_list,
                                      base::Value::List* failed_list);
  // void ExecPauseAllOperate(base::Value::List* success_list,
  //                          base::Value::List* failed_list);
  void ExecDeleteCategoryOperate(nas::TaskCategory task_category,
                                 bool is_delete_local_file,
                                 base::Value::List* success_list,
                                 base::Value::List* failed_list);
  DownloadErrorCode ExecRetryOperate(const std::string& task_id);
  void ExecCleanCategoryOperate(nas::TaskCategory task_category,
                                base::Value::List* success_list,
                                base::Value::List* failed_list);
  DownloadErrorCode ExecRestoreOperate(const std::string& task_id);
  std::vector<TaskBasePtr> SortTask(nas::SortStyle sort_style,
                                    bool is_ascending);
  bool IsCanStart(TaskBasePtr task);

  std::string IsTaskExisted(const TaskParamPtr task_param);

  void BuildMessageData(const std::string& task_id,
                        DownloadErrorCode err,
                        const TaskOperation& operation,
                        base::Value::List* success_list,
                        base::Value::List* failed_list);

  void CheckTaskAndGetTaskInfo(DownloadStatisticsInfo* statistics_info,
                               base::Value::Dict* msg);

  void CheckInvalidTask(TaskBasePtr task);
  void UpdateStatisticsInfo(base::Value::Dict* statistics_json_dict);
  DownloadErrorCode PauseTask(TaskBasePtr task);

  static PfnOrder GetOrderFunc(nas::SortStyle style, bool asc);

  NasThread* GetThread() const;
  void OnParseComplete(const std::string& task_id, const std::string& result);
  bool CheckTaskCountLimit(std::vector<TaskBasePtr>& task_queue);
  void SetTaskContextField(TaskBasePtr task);
  DownloadErrorCode TryResume(TaskBasePtr task);
  DownloadErrorCode ExecMagnet(TaskBasePtr task);
  DownloadErrorCode ExecTorrent(TaskBasePtr task);
  DownloadErrorCode ExecNoneBt(TaskBasePtr task);
  DownloadErrorCode CreateSavePath(const std::string& save_path);
  DownloadErrorCode IsProtectedPath(const std::string& save_path);
  DownloadErrorCode HasWriteAccess(const std::string& save_path);

 private:
  std::string user_id_;
  std::map<std::string, TaskBasePtr> task_list_;
  UserInfoPtr user_info_ = nullptr;
  // NasThread* user_thread_ = nullptr;
  std::shared_ptr<base::RepeatingTimer> publish_timer_ = nullptr;
  TaskComparator comparator_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_ = nullptr;
};
using UserSelfMgrPtr = std::shared_ptr<UserSelfMgr>;
}  // namespace nas
#endif