## nas thread 使用

### 头文件
``` C++
#include "nas/common/nas_thread.h"
```

### BUILD.gn 依赖
``` gn
"//nas/common:nas_thread"
```

### demo
``` C++
  nas::NasThread t1("name1");
  nas::NasThread t2("name1"); // failed, 已存在同名线程
  nas::NasThread t3("name2");
  nas::NasThread t4("");
  nas::NasThread t5(nullptr);

  LOG(INFO) << "" << t1.Id() << ", " << t1.Name();
  LOG(INFO) << "" << t2.Id() << ", " << t2.Name();
  LOG(INFO) << "" << t3.Id() << ", " << t3.Name();
  LOG(INFO) << "" << t4.Id() << ", " << t4.Name();
  LOG(INFO) << "" << t5.Id() << ", " << t5.Name();

  DCHECK(t1.Valid());
  DCHECK(!t2.Valid());
  DCHECK(t3.Valid());
  DCHECK(!t4.Valid());
  DCHECK(!t5.Valid());
  DCHECK(nas::NasThread::GetThread(nullptr) == nullptr);
  DCHECK(nas::NasThread::GetThread("") == nullptr);
  DCHECK(nas::NasThread::GetThread("1") == nullptr);
  DCHECK(nas::NasThread::GetThread("name1") != nullptr);
  DCHECK(nas::NasThread::GetThread("name2") != nullptr);
  DCHECK(nas::NasThread::GetThread(t1.Id()) != nullptr);
  DCHECK(nas::NasThread::GetThread(t2.Id()) == nullptr);
  DCHECK(nas::NasThread::GetThread(t3.Id()) != nullptr);
  DCHECK(nas::NasThread::GetThread(t4.Id()) == nullptr);
  DCHECK(nas::NasThread::GetThread(t5.Id()) == nullptr);

  nas::NasTimerId timer_id1 = 0;
  nas::NasTimerId timer_id2 = 0;
  // 也可通过 id Get
  if (auto t = nas::NasThread::GetThread("name1")) {
    timer_id1 = t->AddTimer([]() { LOG(INFO) << "timer 1 test"; }, 1000);
    timer_id2 = t->AddTimer([]() { LOG(INFO) << "timer 2 test"; }, 5000);
  }

  // stop timer
  if (auto t = nas::NasThread::GetThread("name1")) {
    t->StopTimer(timer_id1, false);
  }

  // restart timer
  if (auto t = nas::NasThread::GetThread("name1")) {
    t->RestartTimer(timer_id1);
  }

  // remove timer
  if (auto t = nas::NasThread::GetThread("name1")) {
    t->StopTimer(timer_id1, true);
  }

  // post task
  if (auto t = nas::NasThread::GetThread("name1")) {
    t->PostTask(
        base::BindLambdaForTesting([]() { LOG(INFO) << "post task test"; }));
    // or
    // t->PostTask(base::BindOnce(xxclass::fun, base::Unretained(this)));
  }
```