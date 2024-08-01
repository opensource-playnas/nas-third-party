/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-18 11:09:41
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_USER_INFO_H_
#define NAS_DOWNLOAD_SERVER_SRC_USER_INFO_H_

#include "base/synchronization/lock.h"
#include "nas/common/context_field.h"
#include "nas/download_service/third_party/download_public_define.h"
#include "utils/save_path_queue.h"

namespace nas {
class UserInfo {
 public:
  UserInfo(const std::string& user_id);
  ~UserInfo() = default;
  UserInfo(const UserInfo& other);

  void ParseAndSubscribe(const std::string& user_id);

  void SetDownloadStatisticsInfo(const DownloadStatisticsInfo& statistics_info);
  DownloadStatisticsInfo GetDownloadStatisticsInfo();

  void SetContextField(const ContextField& context_field);
  ContextField GetContextField();

  void ParseUserSettings(const std::string& user_id);

  uint32_t GetTaskLimitCount();
  bool IsPrioritySmallLimitTask();
  int64_t GetSmallTaskSize();

  void OnSubscribeCallbackResult(const std::string& key,
                                 const std::string& value);
  void OnSubscribeComplete(bool success);
  void SubcribeRedis();
  void AddSavePath(const std::string& path);

 private:
  DownloadStatisticsInfo statistics_info_;
  ContextField context_field_;
  base::Lock lock_;
  std::atomic<uint32_t> task_limit_count_ = 5;  // 同时下载最大任务数
  std::atomic<bool> priority_small_task_switch_ = false;  // 优先下载小任务开关
  std::atomic<int64_t> small_task_size_ = 1024 * 30;  // 小任务 30720kb
  std::string user_id_;
  // SavePathQueue queue_;
};
using UserInfoPtr = std::shared_ptr<UserInfo>;
}  // namespace nas
#endif  // NAS_DOWNLOAD_SERVER_SRC_USER_INFO_H_