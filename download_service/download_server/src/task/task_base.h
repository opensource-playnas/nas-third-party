/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#ifndef NAS_DOWNLOAD_SERVER_TASK_BASE_H_
#define NAS_DOWNLOAD_SERVER_TASK_BASE_H_

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/threading/thread.h"
#include "base/timer/timer.h"

#include "nas/common/context_field.h"
#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"
#include "nas/download_service/download_server/src/config/system_cloud_config.h"
#include "nas/download_service/download_server/src/db/download_store_helper.h"
#include "nas/download_service/download_server/src/download_helper/download_interface_helper.h"
#include "nas/download_service/download_server/src/utils/utils.h"
#include "task_common_define.h"
#include "task_info.h"

namespace nas {

class TaskBase : public std::enable_shared_from_this<TaskBase> {
 public:
  TaskBase(TaskParamPtr param, const ContextField& context_field);
  virtual ~TaskBase();

  // 开始任务
  virtual DownloadErrorCode Start() = 0;
  // 解析任务
  virtual bool Parse() = 0;
  // 更新进度信息
  virtual void UpdateProgressInfo(base::Value::Dict* result) = 0;
  // 获取下载实现接口指针
  virtual DownloadTaskInterface* GetDownloadTaskImp() = 0;
  // 从数据库恢复
  virtual void ResumeDataFromDb(TaskInfoPtr task_info) = 0;
  // 检查条件
  virtual DownloadErrorCode CheckCondition() = 0;
  virtual int SyncParse(ParseResult* result) = 0;

  virtual bool Pause();
  virtual bool Resume();
  virtual bool Retry();
  virtual bool Cancel(bool is_delete_local_file);
  virtual bool DeleteToRecycleBin();
  virtual bool Restore();
  virtual bool ExceCancel(bool is_delete_local_file, bool real_delete);

  TaskParamPtr GetTaskParam();
  void SetTaskStatus(const TaskStatus& status);
  TaskStatus GetTaskStatus();
  void SetContextField(const ContextField& context_field);
  ContextField GetContextField();
  DownloadType GetDownloadType() { return task_info_->GetTaskParam()->type; };
  TaskCategory GetTaskCategory();

  TaskInfoPtr GetTaskInfo();

  bool IsStarted() { return task_info_->IsStarted(); }

  bool IsValid() { return task_info_->IsValid(); }

  bool IsDownloadFinished();

  bool DeleteLocalFile();

  // 根据任务状态及错误码来判断任务失败情况
  bool CheckTaskIsFailed();

  // 更新并推送失败信息
  void UpdateAndPublishFailedInfo();

  // 任务是否有效：检查下载的文件是否存在，不存在即无效
  bool Valid();

  // 是否解析完成
  bool IsParseFinish();

  void SetParseFinish(bool parse_finish) { parse_finished_ = parse_finish; }

  std::string GetTaskHash();

 public:
  void PublishTaskBaseInfo();
  void PublishFailedTaskInfo();
  void PublishFinishedTaskInfo();
  void PublishInvalidTaskInfo();

  base::Value::Dict BuildDownloadingData();
  base::Value::Dict BuildMessageData(const TaskOperation& operation);
  bool IsResumeFromDb(const TaskParamPtr param);

 protected:
  void SendTaskEndMessage(bool success, const std::string& task_name);
  base::Value::Dict GetFileList();
  void SetCreateTimeStamp();

 protected:
  std::shared_ptr<base::RepeatingTimer> publish_timer_ = nullptr;
  TaskInfoPtr task_info_ = nullptr;
  ContextField context_field_;
  std::string
      converted_source_;  // 将下载源链接source进行转换，目前暂时用于bt种子文件转成磁力链接
  bool parse_finished_ = false;
  TaskStatus pre_status_ = TaskStatus::kWaitting;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_ = nullptr;
};  // NAS_DOWNLOAD_SERVER_TASK_BASE_H_
using TaskBasePtr = std::shared_ptr<TaskBase>;
}  // namespace nas

#endif