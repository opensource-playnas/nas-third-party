/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_MULE_TASK_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_MULE_TASK_H_

#include "link_task.h"

namespace nas {
class MuleTask : public LinkTask {
 public:
  MuleTask(TaskParamPtr param, const ContextField& context_field);
  virtual ~MuleTask();

  int SyncParse(ParseResult* result) override;
  DownloadErrorCode CheckCondition() override;
 
  DownloadTaskInterface * GetDownloadTaskImp() override;
  bool Resume() override;
  
 private:
  void SetDownloadParam(const TaskParamPtr param);

 protected:
  MuleDownloadTask* task_impl_ = nullptr;
};
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_MULE_TASK_H_