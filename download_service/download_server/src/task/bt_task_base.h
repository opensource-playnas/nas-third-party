
#ifndef NAS_DOWNLOAD_SERVER_SRC_TASK_BT_TASK_BASE_H_
#define NAS_DOWNLOAD_SERVER_SRC_TASK_BT_TASK_BASE_H_

#include "task_base.h"

namespace nas {
enum TorrentState {
  Unknown = -1,

  ForcedDownloading,
  Downloading,
  ForcedDownloadingMetadata,
  DownloadingMetadata,
  StalledDownloading,

  ForcedUploading,
  Uploading,
  StalledUploading,

  CheckingResumeData,
  QueuedDownloading,
  QueuedUploading,

  CheckingUploading,
  CheckingDownloading,

  PausedDownloading,
  PausedUploading,

  Moving,

  MissingFiles,
  Error
};

class BtTaskBase : public TaskBase {
 public:
  BtTaskBase(TaskParamPtr param, const ContextField& context_field);
  virtual ~BtTaskBase();
  DownloadErrorCode Start() override;
  DownloadErrorCode CheckCondition() override;
  void UpdateProgressInfo(base::Value::Dict* result) override;
  bool Parse() override;
  int SyncParse(ParseResult* result) override;
  void ResumeDataFromDb(TaskInfoPtr task_info) override;
  DownloadTaskInterface* GetDownloadTaskImp() override;
  bool Resume() override;

  // 种子文件转磁力链接
  virtual void CreateMagnetUri() = 0;

 public:
  // 解析种子
  bool ParseTorrent();
  // 更新做种中信息
  void UpdateSeedingInfo(base::Value::Dict* result);
  // 检查做种限制
  bool CheckSeedingLimits();
  // 获取上传速度
  int GetUploadRate();

  static void OnTorrentAdded(void* context);

 protected:
   void UpdateAndPublishTorrentInfo();
   
 private:
  void SetDownloadParam();
  BtDownloadTask* GetBtDownloadTaskImpl(DownloadType type);
  void ReleaseBuf(const char* buf);
  void SetFileList(const char* torrent_list);
  const char* GetTorrentList();
  void ModifyTorrentPath(TaskParamPtr param);


 protected:
  BtDownloadTask* task_impl_ = nullptr;
  TorrenFileItemSelect* item_select_list_ = NULL;
  // scoped_refptr<base::SingleThreadTaskRunner> task_runner_ = nullptr;
};
using BtTaskBasePtr = std::shared_ptr<BtTaskBase>;
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_TASK_BT_TASK_BASE_H_