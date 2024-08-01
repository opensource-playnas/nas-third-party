/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:53:38
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_CURL_TASK_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_CURL_TASK_H_

#include "link_task.h"

namespace nas {
class CurlTask : public LinkTask {
 public:
  CurlTask(TaskParamPtr param, const ContextField& context_field);
  ~CurlTask () override;

  bool Resume() override;
  bool Retry() override;
  bool ResumeFromDb() override;
  int SyncParse(ParseResult* result) override;


  DownloadTaskInterface * GetDownloadTaskImp() override;

 private:
  void SetDownloadParam(const TaskParamPtr param);
  void ReStart();

 protected:
  CurlDownloadTask* task_impl_ = nullptr;
};
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_CURL_TASK_H_