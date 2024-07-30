// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-08-15 10:25:13

#include "nas_thread.h"

#include <map>
#include <vector>

#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "base/test/bind.h"

namespace nas {

struct NasThreadData {
  NasThreadData(const char* name) : thread(name) {}
  base::Thread thread;
  static const std::string kEmptyThreadName;
};

const std::string NasThreadData::kEmptyThreadName = "";
// 保存所有的message loop
static base::Lock all_thread_lock;
static std::vector<NasThread*> all_thread;

NasThread::NasThread(const char* name) {
  Init(name, base::MessagePumpType::DEFAULT);
}

NasThread::NasThread(const char* name, base::MessagePumpType type) {
  Init(name, type);
}

NasThread::~NasThread() {
  Uninit();
}

bool NasThread::Init(const char* name, base::MessagePumpType type) {
  if (!name || strlen(name) == 0) {
    return false;
  }

  if (GetThread(name)) {
    LOG(WARNING) << "exist same name thread";
    return false;
  }

  data_ = std::make_unique<NasThreadData>(name);
  data_->thread.StartWithOptions(base::Thread::Options(type, 0));

  {
    base::AutoLock lock(all_thread_lock);
    all_thread.push_back(this);
  }
  return true;
}

void NasThread::Uninit() {
  if (!data_) {
    return;
  }

  data_->thread.Stop();
  data_.reset();

  {
    base::AutoLock lock(all_thread_lock);
    auto it = find(all_thread.begin(), all_thread.end(), this);
    if (it != all_thread.end()) {
      all_thread.erase(it);
    }
  }
}

void NasThread::Stop() {
  Uninit();
}

bool NasThread::Valid() const {
  return data_ && data_->thread.IsRunning();
}

bool NasThread::PostTask(base::OnceClosure task) {
  return Valid()
             ? data_->thread.task_runner()->PostTask(FROM_HERE, std::move(task))
             : false;
}

scoped_refptr<base::SingleThreadTaskRunner> NasThread::TaskRunner() {
  return Valid() ? data_->thread.task_runner() : nullptr;
}

bool NasThread::IsCurrentThread() const {
  return Valid() ? data_->thread.task_runner()->RunsTasksInCurrentSequence()
                 : false;
}

base::PlatformThreadId NasThread::Id() const {
  return Valid() ? data_->thread.GetThreadId() : base::PlatformThreadId();
}

const std::string& NasThread::Name() const {
  return Valid() ? data_->thread.thread_name()
                 : NasThreadData::kEmptyThreadName;
}

NasThread* NasThread::GetThread(base::PlatformThreadId thread_id) {
  // 加入到vector中的, data_一定不为nullptr
  base::AutoLock lock(all_thread_lock);
  for (auto& nt : all_thread) {
    if (nt && nt->data_->thread.GetThreadId() == thread_id) {
      return nt;
    }
  }
  return nullptr;
}

NasThread* NasThread::GetThread(const char* name) {
  if (!name || strlen(name) == 0)
    return nullptr;
  // 加入到vector中的, data_一定不为nullptr
  base::AutoLock lock(all_thread_lock);
  for (auto& nt : all_thread) {
    if (nt && nt->data_->thread.thread_name() == name) {
      return nt;
    }
  }
  return nullptr;
}

NasThread* NasThread::GetCurrentThread() {
  return GetThread(base::PlatformThread::CurrentId());
}

}  // namespace nas