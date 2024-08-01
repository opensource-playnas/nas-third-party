/*
 * @Description:
 * @copyright 2024 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2024-01-02 17:34:49
 */

#include "task_ref_count.h"

namespace nas {
TaskRefCount* TaskRefCount::GetInstance() {
  return base::Singleton<TaskRefCount>::get();
}

TaskRefCount::TaskRefCount() {}

TaskRefCount::~TaskRefCount() {}

void TaskRefCount::AddRef(const std::string& task_hash) {
  base::AutoLock lock(lock_);
  auto iter = task_ref_.find(task_hash);
  if (iter != task_ref_.end()) {
    iter->second++;
  } else {
    task_ref_[task_hash] = 1;
  }
}

bool TaskRefCount::Release(const std::string& task_hash,
                           ReleaseCallback callback) {
  base::AutoLock lock(lock_);
  auto iter = task_ref_.find(task_hash);
  if (iter != task_ref_.end()) {
    iter->second--;
    if (iter->second <= 0) {
        bool ret =  callback(true);
        if (ret) {
          task_ref_.erase(task_hash);
        } 
        return ret;
    }
  }
  return callback(false);
}

}  // namespace nas
