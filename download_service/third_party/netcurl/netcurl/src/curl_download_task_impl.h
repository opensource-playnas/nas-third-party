/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#ifndef NAS_THIRD_PARTY_DOWNLOAD_CURL_CURLDLL_CURL_DOWNLOAD_TASK_IMPL_H_
#define NAS_THIRD_PARTY_DOWNLOAD_CURL_CURLDLL_CURL_DOWNLOAD_TASK_IMPL_H_

#include <string>
#include <mutex>
#include <filesystem>

#include "curl_task_public_define.h"
#include "network_task.h"
#include "http_download_task.h"
#include "parse_task.h"

class CurlDownloadTaskImpl : public CurlDownloadTask, public nas::NetworkTask::Listener {
 public:
  CurlDownloadTaskImpl() = default;
  ~CurlDownloadTaskImpl();

  void InitTask();
public:
  DownloadErrorCode Start() override;
  bool Pause() override;
  bool Resume(bool is_failed) override;
  bool Cancel(bool is_delete_local_file) override;
  void SetParam(const DownloadParam& param) override;
  bool GetProgressInfo(DownloadProgressInfo* info) override;
  int ParseLink(const char* link, ParseResult* result) override;
  bool ResumeBreakPoint() override;
  void TaskUpdate(nas::NetworkTask::Status task_status,
                          const nas::NetworkTask::Progress* task_progress,
                          const nas::NetworkTask::Result* task_result) override;

  bool Valid(bool is_finished) override;
  void SetMaxDownloadSpeed(int64_t max_download_speed) override;
  void Reset() override;
  const char* GetTaskHash() override;
  void ReleaseBuf(const char* buf) override;
  void TryReStart() override;

 private:
   std::string GetFileNameFromUrl(const std::string& url);
   void RenameTempFile();
   std::filesystem::path GetTempFilePath();

   bool DeleteFinalFinishFile();
   bool IsFinished();
   bool TempFileExist();
   bool FinalFileExist();
private:
  DownloadParam param_;
  std::shared_ptr<nas::HttpDownloadTask> http_task_ = nullptr;
  FileBaseInfo file_base_info_;
  DownloadProgressInfo download_progress_info_;
  std::mutex mutex_;
  bool is_started_ = false;
  std::filesystem::path tmp_file_path_;
  std::filesystem::path final_file_path_;
  std::atomic<int64_t> max_download_speed_ = 0;   // ×ª³É×Ö½Ú/s
  std::chrono::steady_clock::time_point last_pause_time_;
  bool is_http_task_resume_ = false;
  double last_progress_ = 0;
  int count_ = 0;
};

#endif