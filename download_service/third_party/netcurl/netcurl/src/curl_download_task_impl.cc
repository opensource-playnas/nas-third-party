/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#include "curl_download_task_impl.h"
#include <algorithm>
#include <math.h>
#include <codecvt>
#include <filesystem>
#include <locale>
#include <string>
#include <regex>

#include "curl_task_interface.h"

static const double kDeltaValue = 0.0001;

void CurlDownloadTaskImpl::SetParam(const DownloadParam& param) {
  param_ = param;
  memcpy(file_base_info_.file_name, param.file_name, sizeof(param.file_name));
  tmp_file_path_ = GetTempFilePath(); 
  InitTask();
}

CurlDownloadTaskImpl::~CurlDownloadTaskImpl() {

}

void CurlDownloadTaskImpl::InitTask() {
  http_task_ = std::make_shared<nas::HttpDownloadTask>();
  //http_task_->SetTaskId(param_.id);
  http_task_->Init(param_.url);
  http_task_->SetDownloadPath(tmp_file_path_.u8string());
  http_task_->SetMaxDownloadSpeed(max_download_speed_);
  http_task_->SetListener(this);
  download_progress_info_.err_code = DownloadErrorCode::kNormal;
}

DownloadErrorCode CurlDownloadTaskImpl::Start() {
  DownloadErrorCode ret = DownloadErrorCode::kNormal;

  do {
    if (!http_task_) {
      ret = DownloadErrorCode::kCreateCurlTaskFailed;
      break;
    }

    ret = http_task_->CreateTempFile();
    if (ret) {
      break;
    }

    http_task_->Start();
    is_started_ = true;

  } while (0);

  return ret;
}

bool CurlDownloadTaskImpl::Pause() {
  if (!http_task_) {
    return false;
  }

  http_task_->Pause();
  last_pause_time_ = std::chrono::steady_clock::now();
  return true;
}

bool CurlDownloadTaskImpl::Resume(bool is_failed) {
  if (!is_started_) {
    return ResumeBreakPoint();
  }

  // 失败说明重试中
  if (is_failed) {
    if (ResumeBreakPoint()) {
      download_progress_info_.err_code = DownloadErrorCode::kNormal;
      return true;
    }

    return false;
  }

  if (!http_task_) {
    return false;
  }

  http_task_->Resume();
  is_http_task_resume_ = true;
  return true;
}

bool CurlDownloadTaskImpl::Cancel(bool is_delete_local_file) {
  bool ret = false;
  do {
    if (!http_task_) {
      break;
    }

    if (is_delete_local_file && !DeleteFinalFinishFile()) {
      break;
    }

    return http_task_->Cancel(is_delete_local_file);
  } while (0);
  return ret;
}

bool CurlDownloadTaskImpl::GetProgressInfo(DownloadProgressInfo* info) {
  std::lock_guard<std::mutex> lock(mutex_);

  memcpy(info->file_name, file_base_info_.file_name,
         sizeof(file_base_info_.file_name));
  memcpy(download_progress_info_.file_name, file_base_info_.file_name,
         sizeof(file_base_info_.file_name));

  // 如果当前对象任务进度为0，但外面调用方的进度不为0，说明是重启进程，需要重新赋值给该对象
  if (fabs(download_progress_info_.total_progress - 0) < kDeltaValue &&
      info->total_progress != 0) {
    download_progress_info_.total_progress = info->total_progress;
  }

  bool equal = fabs(last_progress_ - download_progress_info_.total_progress) < kDeltaValue;
  // 做个修正：5秒内的进度没变化，将速度置为0
  if (!equal) {
    last_progress_ = download_progress_info_.total_progress;
    count_ = 0;
  } else if (count_ > 5) {
    download_progress_info_.download_speed = 0;
  }
  count_++;

  memcpy(info, &download_progress_info_, sizeof(DownloadProgressInfo));
  info->err_code = download_progress_info_.err_code;
  return true;
}

int CurlDownloadTaskImpl::ParseLink(const char* link, ParseResult* result) {
  std::shared_ptr<nas::ParseTask> parse_task =
      std::make_shared<nas::ParseTask>();
  int ret = parse_task->Parse(link, &result->file_info.file_size);
  std::string file_name = file_base_info_.file_name;
  if (file_name.empty()) {
    file_name = GetFileNameFromUrl(param_.url);
  }
  memcpy(file_base_info_.file_name, file_name.c_str(), file_name.length());
  memcpy(result->file_info.file_name, file_base_info_.file_name,
         sizeof(file_base_info_.file_name));
  return ret;
}

bool CurlDownloadTaskImpl::ResumeBreakPoint() {
  if (http_task_) {
    std::error_code err;
    if (http_task_->CreateTempFile() == DownloadErrorCode::kNormal &&
        std::filesystem::exists(tmp_file_path_, err)) {
      http_task_->Reset();
      http_task_->SetListener(this);
      http_task_->Start();
      is_started_ = true;
      return true;
    }
  }

  return false;
}

void CurlDownloadTaskImpl::TaskUpdate(
    nas::NetworkTask::Status task_status,
    const nas::NetworkTask::Progress* task_progress,
    const nas::NetworkTask::Result* task_result) {
  if (task_result) {
    std::lock_guard<std::mutex> lock(mutex_);
    download_progress_info_.err_code = DownloadErrorCode::kNormal;
    if (task_result->curl_code != CURLE_OK) {
      download_progress_info_.err_code =
          (DownloadErrorCode)(task_result->curl_code +
                              DownloadErrorCode::kPerformCurlStartErrorCode);
    } else if (task_result->http_code < HttpCode::kOk || task_result->http_code >= HttpCode::kBadRequest) {
      download_progress_info_.err_code = static_cast<DownloadErrorCode>(DownloadErrorCode::kHttpStartErrorCode + task_result->http_code);
      // 转换失败，错误码设置为kPerformCurlFailed
      if (download_progress_info_.err_code <= DownloadErrorCode::kHttpStartErrorCode || download_progress_info_.err_code > kHttpError504) {
        download_progress_info_.err_code = DownloadErrorCode::kPerformCurlFailed;
      }
      if (download_progress_info_.total_progress == 100) {
        download_progress_info_.total_progress = 0;
      }
    } else {
      RenameTempFile();
    }
  }

  if (task_progress) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (task_progress->percent < 100) {
      download_progress_info_.total_progress = task_progress->percent;
    }
    
    download_progress_info_.download_speed = task_progress->current_speed_byte;
  }
}

bool CurlDownloadTaskImpl::Valid(bool is_finished) {
  if (is_finished) {
    return FinalFileExist();
  } else {
    return TempFileExist();
  }
}

void CurlDownloadTaskImpl::SetMaxDownloadSpeed(int64_t max_download_speed) {
  max_download_speed_ = max_download_speed * 1024;  // 转成字节/s
  if (http_task_) {
    http_task_->SetMaxDownloadSpeed(max_download_speed_);
  }
}

std::string CurlDownloadTaskImpl::GetFileNameFromUrl(const std::string& url) {
  std::regex uri_regex(R"(^(?:[^:/?#]+:)?(?://([^/?#]*))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?)");
  std::smatch uri_match;
  std::string file_name;
  do 
  {
    if (!std::regex_match(url, uri_match, uri_regex)) {
      break;
    }

    // 提取路径
    std::string path = uri_match[2];

    // 查找路径中最后一个 '/' 的位置
    size_t last_slash_pos = path.find_last_of('/');

    // 如果没有找到 '/', 则文件名就是整个路径
    if (last_slash_pos == std::string::npos) {
      file_name = path.empty() ? "index.html" : path;
      break;
    }

    // 提取 '/' 后面的部分作为文件名
    file_name = path.substr(last_slash_pos + 1);

    // 如果文件名为空，返回 "index.html"
    if (file_name.empty()) {
      file_name = "index.html";
    }
  } while (0);
 

  return file_name;
}

void CurlDownloadTaskImpl::RenameTempFile() {
  std::string file_path = param_.save_path;
  file_path.append("/");
  file_path.append(file_base_info_.file_name);

  std::filesystem::path u8_name =
      std::filesystem::u8path(file_base_info_.file_name);
  std::string u8_stem = u8_name.stem().u8string();

  int name_suffix = 1;
  std::filesystem::path new_u8_file_path = std::filesystem::u8path(file_path);
  std::error_code err;
  while (std::filesystem::exists(new_u8_file_path, err)) {
    std::string extension = new_u8_file_path.extension().u8string();
    std::string new_file_name =
        u8_stem + " (" + std::to_string(name_suffix) + ")" + extension;
    std::filesystem::path path = std::filesystem::u8path(new_file_name);
    new_u8_file_path = new_u8_file_path.replace_filename(path);
    name_suffix++;
  }
  final_file_path_ = new_u8_file_path;

  if (std::filesystem::exists(tmp_file_path_, err) &&
      !std::filesystem::exists(final_file_path_, err)) {
    std::error_code ec;
    std::filesystem::rename(tmp_file_path_, final_file_path_, ec);
    if (ec) {
      download_progress_info_.err_code =
          DownloadErrorCode::kRenameTempFileFailed;
    } else {
      std::string final_name = final_file_path_.filename().u8string().c_str();
      memcpy(file_base_info_.file_name, final_name.c_str(), final_name.length());
      download_progress_info_.total_progress = 100;
    }
    return;
  }
  download_progress_info_.err_code = DownloadErrorCode::kRenameTempFileFailed;
}

std::filesystem::path CurlDownloadTaskImpl::GetTempFilePath() {
  std::string file_name = file_base_info_.file_name;
  if (file_name.empty()) {
    file_name = GetFileNameFromUrl(param_.url);
  }
  std::filesystem::path u8_name = std::filesystem::u8path(file_name);
  file_name = u8_name.u8string();
  memcpy(file_base_info_.file_name, file_name.c_str(), file_name.length());

  // 临时文件名格式为：文件名.task_id.nas.tmp
  // e.g. file_name 为
  // WeChatSetup.exe，则落地的临时文件为WeChatSetup.exe.task_id.nas.tmp

  std::string tmp_file_name = file_name;
  tmp_file_name.append(".");
  tmp_file_name.append(param_.id);
  tmp_file_name.append(".nas.tmp");

  // 获取临时文件路径
  std::string file_path = param_.save_path;
  file_path.append("/");
  file_path.append(tmp_file_name);
  std::filesystem::path u8_file_path = std::filesystem::u8path(file_path);
  return u8_file_path;
}

bool CurlDownloadTaskImpl::DeleteFinalFinishFile() {
  std::error_code err;
  if (FinalFileExist() && !std::filesystem::remove(final_file_path_, err)) {
    return false;
  }

  return true;
}

bool CurlDownloadTaskImpl::IsFinished() {
  return download_progress_info_.total_progress == 100 ? true : false;
}

bool CurlDownloadTaskImpl::TempFileExist() {
  if (tmp_file_path_.empty()) {
    tmp_file_path_ = GetTempFilePath();
  }
  std::error_code err;
  if (!std::filesystem::exists(tmp_file_path_, err)) {
    return false;
  }
  return true;
}

bool CurlDownloadTaskImpl::FinalFileExist()
{
  if (final_file_path_.empty()) {
    std::string file_path = param_.save_path;
    file_path.append("/");
    file_path.append(file_base_info_.file_name);
    final_file_path_ = std::filesystem::u8path(file_path);
  }

  std::error_code err;
  if (!std::filesystem::exists(final_file_path_, err)) {
    return false;
  }

  return true;
}


void CurlDownloadTaskImpl::Reset() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    download_progress_info_.total_progress = 0;
    download_progress_info_.download_speed = 0;
    download_progress_info_.err_code = DownloadErrorCode::kNormal;
  }

  if (http_task_) {
    http_task_->Reset();
  }
}

const char* CurlDownloadTaskImpl::GetTaskHash() {
  const char* hash = param_.url;
  int length = strlen(hash);
  char* buf = new char[length + 1];
  memset(buf, 0, length);
  memcpy(buf, hash, length);
  buf[length] = '\0';
  return buf;
}

void CurlDownloadTaskImpl::ReleaseBuf(const char* buf) {
  if (buf) {
    delete[] buf;
  }
}

void CurlDownloadTaskImpl::TryReStart()
{
  if (!is_http_task_resume_) {
    is_http_task_resume_ = false;
    return;
  }

  auto now_time = std::chrono::steady_clock::now();
  auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(now_time - last_pause_time_).count();
  // 如果暂停时长大于2分钟，就重新下载
  if (time_elapsed > 120) {
    // 长时间暂停后，再恢复，会有部分数据不完整，
    // 这个时候必须清理掉task_curl_ptr_，即使重置文件大小也不行（会导致最终下载的文件大小与实际不符）
    http_task_.reset();
    InitTask();
    Start();
  }
}
