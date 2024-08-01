/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 15:22:38
 */

#include <algorithm>
#include <chrono>  // std::chrono::milliseconds
#include <codecvt>
#include <filesystem>
#include <locale>

#include "http_download_task.h"

namespace nas {
HttpDownloadTask::HttpDownloadTask() {}

HttpDownloadTask::~HttpDownloadTask() {
  cancel_.store(true);
  WaitThreadQuit();
}

void HttpDownloadTask::SetDownloadPath(const std::string& file_path) {
  if (file_path.empty()) {
    return;
  }
  download_file_path_ = file_path;
}

void HttpDownloadTask::Perform() {
  int retry_count = setparam_retry_count;
  do {
    // 设置最大下载速度（以字节/秒为单位), 如果是0则表示不限速
    curl_off_t max_speed = max_download_speed_.load();
    if (max_speed != 0) {
      curl_easy_setopt(task_curl_ptr_, CURLOPT_MAX_RECV_SPEED_LARGE, max_speed);
    }
    curl_easy_setopt(task_curl_ptr_, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(task_curl_ptr_, CURLOPT_WRITEFUNCTION, WriteFuncCallback);
    curl_easy_setopt(task_curl_ptr_, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(task_curl_ptr_, CURLOPT_NOPROGRESS, listener_ ? 0L : 1L);

    if (listener_) {
      curl_easy_setopt(task_curl_ptr_, CURLOPT_XFERINFOFUNCTION,
        ProgressFuncCallback);
      curl_easy_setopt(task_curl_ptr_, CURLOPT_XFERINFODATA, this);
    }

    last_recv_time_ = std::chrono::steady_clock::now();
    last_downloaded_bytes_ = 0;
   
    SetFileSize();
    curl_code_ = curl_easy_perform(task_curl_ptr_);
    if (curl_code_ == CURLE_OK)
      break;

    if (curl_code_ == CURLE_ABORTED_BY_CALLBACK || cancel_.load()) {
      break;
    }

    if (curl_code_ ==  CURLE_PARTIAL_FILE) {
      return;
    }

  } while (--retry_count);

  status_ = curl_code_ == CURLE_OK ? kSuccess : kFail;
  CloseFile();

  if (listener_) {
    Result result;
    UpdateResult(&result);
    listener_->TaskUpdate(status_, nullptr, &result);
  }
}

bool HttpDownloadTask::Cancel(bool is_delete_local_file /*= false*/) {
  bool ret = true;
  do {
    if (!cancel_.load()) {
      cancel_.store(true);
      WaitThreadQuit();
    }
    CloseFile();
    if (is_delete_local_file) {
      ret = DeleteLocalTempFile();
    }

  } while (0);

  return ret;
}

void HttpDownloadTask::SetUrlOpt(CURL* task_curl) {
  if (!task_curl) {
    return;
  }
  curl_easy_setopt(task_curl, CURLOPT_HTTPGET, 1);
}

bool HttpDownloadTask::WriteFileData(void* data,
  size_t size,
  size_t nmemb,
  size_t* real_write_size) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (file_) {
    *real_write_size = fwrite(data, size, nmemb, file_);
    if (*real_write_size < nmemb) {
      return false;
    }
    return true;
  }
  return false;
}

void HttpDownloadTask::CloseFile() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (file_) {
    fclose(file_);
    file_ = nullptr;
  }
}

DownloadErrorCode HttpDownloadTask::CreateTempFile() {
  MakeSureSavePathDir();
  std::lock_guard<std::mutex> lock(mutex_);
  if (file_) {
    return DownloadErrorCode::kNormal;
  }

#ifdef _WINDOWS
  std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
  std::wstring wstr = conv.from_bytes(download_file_path_);
  file_ = _wfopen(wstr.c_str(), L"ab+");
#else
  file_ = fopen(download_file_path_.c_str(), "ab+");
#endif
  if (file_) {
    return DownloadErrorCode::kNormal;
  }

  return DownloadErrorCode::kCreateFileFailed;
}

void HttpDownloadTask::MakeSureSavePathDir() {
  std::filesystem::path u8_file_path =
    std::filesystem::u8path(download_file_path_);
  std::filesystem::path parent_path = u8_file_path.parent_path();
  std::error_code err;
  if (!std::filesystem::exists(parent_path, err)) {
    std::filesystem::create_directory(parent_path, err);
  }
}

bool HttpDownloadTask::DeleteLocalTempFile() {
  std::filesystem::path u8_file_path =
    std::filesystem::u8path(download_file_path_);
  std::error_code err;
  if (std::filesystem::exists(u8_file_path, err) &&
    !std::filesystem::remove(u8_file_path, err)) {
    return false;
  }

  return true;
}

void HttpDownloadTask::SetMaxDownloadSpeed(int64_t max_download_speed) {
  max_download_speed_.store(max_download_speed);
}

void HttpDownloadTask::SetFileSize() {
  std::filesystem::path u8_file_path =
    std::filesystem::u8path(download_file_path_);
  std::error_code err;
  unsigned long long file_size = std::filesystem::file_size(u8_file_path, err);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_RESUME_FROM_LARGE, file_size);
  file_init_size_ = file_size;

  do
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_) {
      return;
    }
    fseek(file_, 0, SEEK_END);
  } while (0);
}

}

