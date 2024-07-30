// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-08-15 10:23:27

#ifndef NAS_COMMON_NAS_THREAD_H_
#define NAS_COMMON_NAS_THREAD_H_

#include <functional>
#include <memory>

#include "base/bind.h"
#include "base/task/single_thread_task_runner.h"
#include "base/threading/thread.h"

namespace nas {

struct NasThreadData;
using NasThreadDataPtr = std::unique_ptr<NasThreadData>;

/*
对 base::thread 简单封装
1 不存在同名线程, 一个 name 对应一个 thread
2 name 和 thread id 都是 thread 的唯一 ID
*/

class NasThread {
 public:
  NasThread(const NasThread&) = delete;
  NasThread& operator=(const NasThread&) = delete;

  // will call Init
  // 构造成功则会 Start 线程
  NasThread(const char* name);
  NasThread(const char* name, base::MessagePumpType type);
  // will call Uninit
  ~NasThread();

  // thread 是否有效
  bool Valid() const;
  // 停止线程, call Uninit
  // 停止之后, 通过名称和id再也查找不到该对象
  void Stop();

  // 默认 FROM_HERE, 如果指定其他 location, 通过task runner post task
  bool PostTask(base::OnceClosure task);
  scoped_refptr<base::SingleThreadTaskRunner> TaskRunner();

  bool IsCurrentThread() const;
  base::PlatformThreadId Id() const;
  const std::string& Name() const;

  // 根据线程 id 获取 thread
  static NasThread* GetThread(base::PlatformThreadId thread_id);
  // 根据 name 获取 thread, 如果name为 null 或 者空, 返回 null
  static NasThread* GetThread(const char* name);
  // 获取当前线程, 如果不存在 返回 null
  static NasThread* GetCurrentThread();

 protected:
  // 初始化 thread, 新开线程
  // 如果已经存在指定 name 的 thread, 返回false
  //     name 不能为 nullptr
  bool Init(const char* name, base::MessagePumpType type);
  // 关闭 thread
  void Uninit();

 private:
  NasThreadDataPtr data_;
};

}  // namespace nas

#endif