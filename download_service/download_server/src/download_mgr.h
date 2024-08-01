/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:12
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_MGR_H_
#define NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_MGR_H_

#include <memory>
#include <queue>

#include "base/threading/thread.h"
#include "base/timer/timer.h"
#include "download_error_desc.h"
#include "nas/common/context_field.h"
#include "nas/common/nas_thread.h"
#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"
#include "nas/download_service/third_party/download_public_define.h"
#include "task/task_base.h"
#include "user_self_mgr.h"

namespace nas {

using TaskMap = std::map<std::string, TaskBasePtr>;
typedef bool (*PfnOrder)(const TaskBasePtr& a, const TaskBasePtr& b);

class DownloadManager : public std::enable_shared_from_this<DownloadManager> {
 public:
  DownloadManager();
  ~DownloadManager();

  void Init();
  void UnInit();
  void Reset(base::WaitableEvent& wait_event);

  void CreateLinkDownloadTask(
      const download::v1::CreateLinkDownloadTaskRequest* request,
      const ContextField& context_field);

  void CreateTorrentDownloadTask(
      const download::v1::CreateTorrentDownloadTaskRequest* request,
      const ContextField& context_field);

  std::string UploadTorrentFile(
      const download::v1::UploadTorrentFileRequest* request);

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

  std::string GetTorrentUploadPath(const ContextField& context_field);

 private:
  void ResumeTasksFromDb();
  UserSelfMgrPtr FindUser(const ContextField& context_field);
  UserSelfMgrPtr AddUser(const ContextField& context_field);
  scoped_refptr<base::SingleThreadTaskRunner> AllocateTaskRunner(
      const ContextField& context_field);
  void CreateDefaultDownloadDirectory(const std::string& user_id);

 private:
  std::shared_ptr<NasThread> work_thread_ = nullptr;
  // key: user; value:用户自身管理指针
  std::map<std::string, UserSelfMgrPtr> user_self_mgr_list_;
  std::vector<std::shared_ptr<NasThread>> user_thread_list_;
};

}  // namespace nas
#endif  // NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_MGR_H_