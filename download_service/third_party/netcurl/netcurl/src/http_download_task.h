/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 15:22:38
 */
#include <mutex>

#include "network_task.h"
#include "download_public_define.h"

namespace nas {
class HttpDownloadTask : public NetworkTask {
 public:
  HttpDownloadTask();
  ~HttpDownloadTask();
  virtual Type TaskType() { return NetworkTask::kHttpDownload; }
  void SetDownloadPath(const std::string& file_path);
  virtual void Perform();
  virtual bool Cancel(bool is_delete_local_file = false);
  DownloadErrorCode CreateTempFile();
  bool DeleteLocalTempFile();
  void SetMaxDownloadSpeed(int64_t max_download_speed);

  void SetFileSize();
protected:
  virtual void SetUrlOpt(CURL* task_curl);
  virtual bool WriteFileData(void* data, size_t size, size_t nmemb, size_t* real_write_size);
  void CloseFile();
  void MakeSureSavePathDir();


private:
  std::string download_file_path_;
  FILE* file_ = NULL;
  std::mutex mutex_;
  std::atomic<int64_t> max_download_speed_ = 0;  // ×Ö½Ú/s
};

}  // namespace nas
