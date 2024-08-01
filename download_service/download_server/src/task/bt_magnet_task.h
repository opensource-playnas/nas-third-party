/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-20 16:04:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_BT_MAGNET_TASK_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_BT_MAGNET_TASK_H_

#include "bt_task_base.h"

namespace nas {
using OnParseCompletedCallback =
    base::OnceCallback<void(const std::string& task_id,
                            const std::string& result)>;

class BtMagnetTask : public BtTaskBase {
 public:
  BtMagnetTask(TaskParamPtr param, const ContextField& context_field);
  bool Parse() override;
  void SetParseCompletedCallback(OnParseCompletedCallback callback);
  OnParseCompletedCallback GetParseCompletedCallback();
  void UpdateFileBaseInfo(const std::string& result);
  static void ParseMangetResult(void* context, const char* result);
  void CreateMagnetUri() override;
  
private:
  OnParseCompletedCallback parse_callback_;
};
using BtMagnetTaskPtr = std::shared_ptr<BtMagnetTask>;
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_BT_MAGNET_TASK_H_