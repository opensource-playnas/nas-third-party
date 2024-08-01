/*
 * @Description:
 * @copyright 2024 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2024-01-02 17:34:45
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_REF_COUNT_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_REF_COUNT_H_

#include <map>

#include "base/callback.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"

namespace nas {
using ReleaseCallback =
    std::function<bool(bool real_delete)>;

class TaskRefCount {
 public:
  static TaskRefCount* GetInstance();
  TaskRefCount(const TaskRefCount&) = delete;
  TaskRefCount& operator=(const TaskRefCount&) = delete;

 public:
  void AddRef(const std::string& task_hash);
  // true代表引用计数减为0
  bool Release(const std::string& task_hash, ReleaseCallback callback);

 private:
  // This object is a singleton:
  TaskRefCount();
  ~TaskRefCount();
  friend struct base::DefaultSingletonTraits<TaskRefCount>;

 private:
  std::map<std::string, int> task_ref_;
  base::Lock lock_;
};
}  // namespace nas
#endif