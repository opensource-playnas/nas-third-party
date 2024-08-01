/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:29
 */

#ifndef NAS_DOWNLOAD_SRC_TASK_TASK_COMMON_DEFINE_H_
#define NAS_DOWNLOAD_SRC_TASK_TASK_COMMON_DEFINE_H_

#include <string>
#include "nas/common/context_field.h"
#include "nas/download_service/download_server/protos/download_server.grpc.pb.h"
#include "nas/download_service/third_party/download_public_define.h"
#include "nas/download_service/download_server/src/download_helper/download_api_interface.h"

namespace nas {
const static double kDValue = 0.0001;
const static char* kTaskBaseInfoCallId = "task_base_info";
const static char* kTaskProgressInfoCallId = "task_progress_info";
const static char* kWaittingTaskInfoCallId = "waitting_task_info";
const static char* kPausedTaskInfoCallId = "paused_task_info";
const static char* kFailedTaskInfoCallId = "failed_task_info";
const static char* kFinishedTaskInfoCallId = "finished_task_info";
const static char* kDeletedTaskInfoCallId = "recyclebin_task_info";
const static char* kInvalidTaskInfoCallId = "invalid_task_info";
const static char* kCompletedDeleteTaskInfoCallId =
    "completed_delete_task_info";

const static char* kTaskControlOperationId = "task_control";
const static char* kTaskCreateOperationId = "task_create";
const static char* kTaskParseCallId = "task_parse";

enum TaskStatus {
  kDownloading = 0,      // 下载中
  kWaitting = 1,         // 等待中
  kPaused = 2,           // 已暂停
  kFailed = 3,           // 失败
  kFinished = 4,         // 已完成
  kDeleted = 5,          // 已删除（可恢复）
  kCompletedDeleted = 6,  // 完全删除
  kParsing = 7,            // 解析中
  kSeeding = 8,            // 做种中
  kChecking = 9            // 校验数据
};

enum TaskCategory {  
  kAll = -2, // 直接删除所有
  kUnkownnTask = -1,     // 未知任务分类
  kDownloadingTask = 0,  // 下载中任务
  kFinishedTask,         //  已完成任务
  kRecyclebinTask,        // 回收站任务
  kSeedingTask,        // 做种中任务
};

enum SortStyle {
  kUnknown = 0,   // 未知
  kCreateTime,    // 按创建时间
  kFinishedTime,  // 按完成时间
  kDeletedTime,   // 按删除时间
  kTaskName,      // 按任务名称
  kTaskSize       // 按任务大小
};

enum TaskOperation {
  kCreate = 0,  
  kParse,  
  kPause,          
  kPauseAll,       
  kResume,          
  kResumeAll,     
  kDelete,         
  kDeleteCategory,         
  kRetry,          
  kRetryAll,        
  kClean,         
  kCleanCategory,          
  kRestore, 
  kRestoreAll 
};

struct TaskTimeStamp {
  TaskTimeStamp(){}
   TaskTimeStamp(const TaskTimeStamp& other)
      :create_time(other.create_time),
      finish_time(other.finish_time),
      delete_time(other.delete_time) {
  }
  int64_t create_time = 0;
  int64_t finish_time = 0;
  int64_t delete_time = 0;
};
using TaskTimeStampPtr = std::shared_ptr<TaskTimeStamp>;

struct TaskParam {
  TaskParam() {};
  TaskParam(const TaskParam& other)
      : id(other.id),
        url(other.url),
        save_path(other.save_path),
        file_name(other.file_name),
        file_size(other.file_size),
        type(other.type),
        converted_url(other.converted_url),
        hash(other.hash),
        is_from_db_resume(other.is_from_db_resume) {}
  std::string id;
  std::string url;
  std::string save_path;
  std::string file_name;
  int64_t file_size = 0;
  DownloadType type = DownloadType::kUnknown;
  std::string converted_url;
  std::string hash;
  bool is_from_db_resume = false; // 是否从db中恢复
  bool is_parse_task = false; // 是否是解析任务
};
using TaskParamPtr = std::shared_ptr<TaskParam>;

struct TorrentTaskParam : public TaskParam {
  std::vector<TorrenFileItemSelect> torrent_item_select_list;
  int selected_count = 0;
  bool is_private_cloud_source = false; // bt来源，只有服务电脑上的种子文件才是私有云源，磁力连接和本地浏览器上传的种子文件都不是
  bool is_download_now = false;  // 是否立即下载
  std::string front_trans_url; // 前端或移动端传递的下载源，添加本地种子文件或私有云种子文件如果添加错误会删除该源对应的路径
};
using TorrentTaskParamPtr = std::shared_ptr<TorrentTaskParam>;

struct FileAttribute {
  std::string file_name; // 任务名称
  int64_t file_size = 0;  // 任务大小
  std::string file_type;  // 任务类型
  std::string local_land_file_name; // 本机落地的文件名
};
using FileAttributePtr = std::shared_ptr<FileAttribute>;

}  // namespace nas

#endif
