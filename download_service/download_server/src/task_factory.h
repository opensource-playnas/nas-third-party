/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-04-18 19:57:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_FACTORY_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_FACTORY_H_

#include "nas/common/context_field.h"
#include "nas/download_service/third_party/download_public_define.h"
#include "task/task_base.h"

namespace nas {

class TaskFactory {
 public:
  TaskFactory();
  ~TaskFactory();
  static TaskBasePtr CreateTask(TaskParamPtr task_param,
                                const ContextField& context_field);

  static DownloadType GetDownloadType(const std::string& url);
  static TorrentTaskParamPtr GetTorrentTaskParam(
      TaskParamPtr task_param,
      const ContextField& context_field);
};
}  // namespace nas
#endif