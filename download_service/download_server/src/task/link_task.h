/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:53:38
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_LINK_TASK_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_LINK_TASK_H_

#include "task_base.h"

namespace nas {
class LinkTask : public TaskBase {
 public:
  LinkTask(TaskParamPtr param, const ContextField& context_field);
  ~LinkTask() override;
  DownloadErrorCode Start() override;
  void UpdateProgressInfo(base::Value::Dict* result) override;
  bool Parse() override;
  void ResumeDataFromDb(TaskInfoPtr task_info) override;
  DownloadTaskInterface* GetDownloadTaskImp() override { return nullptr; }
  int SyncParse(ParseResult* result) override { return 0; }
  DownloadErrorCode CheckCondition() override;
  
 public:
  virtual bool ResumeFromDb() { return true; };

 protected:
  virtual void AssembleDownloadParam(DownloadParam* param,
                                     const TaskParamPtr task_param);
  void SetLocalLandFileName();
};
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_LINK_TASK_H_