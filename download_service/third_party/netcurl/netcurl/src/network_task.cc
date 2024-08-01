/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 15:22:38
 */
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include "network_task.h"
#include "curl_export_define.h"
namespace nas {

NetworkTask::NetworkTask() {
  task_curl_ptr_ = curl_easy_init();
}

NetworkTask::~NetworkTask() {
   if (task_curl_ptr_) {
    curl_easy_cleanup(task_curl_ptr_);
    task_curl_ptr_ = NULL;
  }

  //if (log_file_.is_open()){
  //  log_file_.close();
  //}
}

//static int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userp) {
//  std::ofstream* logfile = static_cast<std::ofstream*>(userp);
//  logfile->write(data, size);
//  logfile->flush(); // 刷新缓冲区，将数据写入磁盘
//
//  return 0;
//}

bool NetworkTask::Init(const char* url) {
  url_ = url;
  curl_easy_setopt(task_curl_ptr_, CURLOPT_URL, url);  // url
  curl_easy_setopt(task_curl_ptr_, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_NOSIGNAL, 1L);

  //std::stringstream filename;

  //// 获取当前时间（精确到毫秒）
  //auto now = std::chrono::system_clock::now();
  //auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  //std::string file_path = "./report/curl_logs";
  //if (!std::filesystem::exists(file_path)) {
  //  std::filesystem::create_directories(file_path);
  //}

  //// 格式化时间戳
  //std::time_t t = std::chrono::system_clock::to_time_t(now);
  //filename << file_path << "/" << "curl_debug_" << std::put_time(std::localtime(&t), "%Y%m%d_%H%M%S") << "_" << std::setw(3) << std::setfill('0') << ms.count() << ".log";

  //log_file_.open(filename.str());
  //if (log_file_.is_open()) {
  //  std::string content = "task_id: ";
  //  content.append(task_id_);
  //  content.append("\n");
  //  log_file_.write(content.c_str(), content.length());
  //  log_file_.flush(); // 刷新缓冲区，将数据写入磁盘
  //}

  //// 启用详细输出
  //curl_easy_setopt(task_curl_ptr_, CURLOPT_VERBOSE, 1L);
  //// 设置调试回调函数和输出文件
  //curl_easy_setopt(task_curl_ptr_, CURLOPT_DEBUGFUNCTION, debug_callback);
  //curl_easy_setopt(task_curl_ptr_, CURLOPT_DEBUGDATA, &log_file_);


  // 忽略SSL
  curl_easy_setopt(task_curl_ptr_, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_SSL_VERIFYHOST, 0);
  // 支持重定向
  curl_easy_setopt(task_curl_ptr_, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");
  curl_easy_setopt(task_curl_ptr_, CURLOPT_REFERER, "https://mirrors.tuna.tsinghua.edu.cn/");
  return true;
}

void NetworkTask::SetTaskId(const char* id) {
  task_id_ = id;
}

void NetworkTask::AddHeader(const char* key, const char* value) {}

bool NetworkTask::SetParam(const char* key, const char* value) {
  return true;
}

bool NetworkTask::SetParam(const char* key, const int& value) {
  return true;
}

void NetworkTask::SetListener(Listener* listener) {
  listener_ = listener;
}

void NetworkTask::Start() {
  if (!task_curl_ptr_) {
    return;
  }

  thread_.reset(new std::thread(&NetworkTask::Perform, this));
}

CURLcode NetworkTask::Pause() {
  CURLcode ret = CURLcode::CURLE_OK;
  if (task_curl_ptr_) {
    ret = curl_easy_pause(task_curl_ptr_, CURLPAUSE_RECV);
  }
  return ret;
}

CURLcode NetworkTask::Resume() {
  CURLcode ret = CURLcode::CURLE_OK;
  if (task_curl_ptr_) {
    ret = curl_easy_pause(task_curl_ptr_, CURLPAUSE_RECV_CONT);
  }
  return ret;
}

bool NetworkTask::Cancel(bool is_delete_local_file /*= false*/) {
  cancel_.store(true);
  WaitThreadQuit();
  return true;
}

void NetworkTask::WaitThreadQuit() {
  if (thread_ && thread_->joinable()) {
    thread_->join();
  }
}

void NetworkTask::Reset() {
  cancel_.store(true);
  WaitThreadQuit();
  thread_ = nullptr;
  cancel_.store(false);
}

bool NetworkTask::GetResponseContent(std::string* content) {
  return true;
}

void NetworkTask::Perform() {
  
}

bool NetworkTask::UpdateProgress(curl_off_t dltotal,
                                 curl_off_t dlnow,
                                 curl_off_t ultotal,
                                 curl_off_t ulnow,
                                 Progress* progress) {
  if (!progress) {
    return false;
  }
  bool is_last = dlnow == dltotal;

  curl_off_t current_time;
  curl_easy_getinfo(task_curl_ptr_, CURLINFO_TOTAL_TIME_T, &current_time);

  if (dltotal) {
    if (first_has_total_value_) {
      first_has_total_value_ = false;
      file_total_size_ = dltotal + file_init_size_;
    }
    if (file_total_size_ == 0) {
      file_total_size_ = dltotal;
    }

    progress->percent = (dlnow + file_init_size_) * 100.0 / file_total_size_;
    progress->total_data_length_byte = file_total_size_;
    progress->recv_data_length_byte = dlnow;
  }

  progress->time = (current_time / 1000000);

  auto now_time = std::chrono::steady_clock::now();
  auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - last_recv_time_).count();

  if (time_elapsed >= 1000) {
    curl_off_t bytes_downloaded = dlnow - last_downloaded_bytes_;
    double speed = static_cast<double>(bytes_downloaded) / (time_elapsed / 1000.0);
    progress->current_speed_byte = speed;

    last_recv_time_ = now_time;
    last_downloaded_bytes_ = dlnow;
  }
  // 当间隔时间太短的时候不需要通知出去，这个时候速度计算是0
  else if (time_elapsed < 1000 && !is_last) {
    return false;
  }

  return true;
}

bool NetworkTask::UpdateResult(Result* result) {
  if (!result) {
    return false;
  }

  long response_code = 0;
  curl_easy_getinfo(task_curl_ptr_, CURLINFO_RESPONSE_CODE, &response_code);
  result->http_code = response_code;

  // 避免下载完成取不到response_code情况导致报错
  if (curl_code_ == CURLE_OK && file_total_size_ > 0 && response_code == 0) {
    result->http_code = 200;
  }
  result->curl_code = curl_code_;

  curl_off_t total_time_ms = 0;
  curl_easy_getinfo(task_curl_ptr_, CURLINFO_TOTAL_TIME_T, &total_time_ms);
  result->request_time = (long long)(total_time_ms / 1000000);

  curl_off_t speed_dowload = 0;
  curl_easy_getinfo(task_curl_ptr_, CURLINFO_SPEED_DOWNLOAD_T, &speed_dowload);
  result->avg_current_speed_byte = (long long)(speed_dowload / 1024);
  return true;
}

size_t NetworkTask::WriteFuncCallback(void* data,
                                      size_t size,
                                      size_t nmemb,
                                      void* network_task_ptr) {
  NetworkTask* this_ptr = (NetworkTask*)network_task_ptr;
  unsigned long data_length = size * nmemb;

  do {
    if (!data || data_length == 0) {
      break;
    }

    if (!this_ptr) {
      break;
    }

    // 如果写入文件失败，可能是磁盘空间不足
    size_t real_write_size = 0;
    if (!this_ptr->WriteFileData(data, size, nmemb, &real_write_size)) {
      return real_write_size;
    }

  } while (false);

  return data_length;
}

int NetworkTask::ProgressFuncCallback(void* network_task_ptr,
                                      curl_off_t dltotal,
                                      curl_off_t dlnow,
                                      curl_off_t ultotal,
                                      curl_off_t ulnow) {
  NetworkTask* this_ptr = (NetworkTask*)network_task_ptr;
  int stop = 1;
  do {
    if (!this_ptr) {
      break;
    }

    if (this_ptr->cancel_) {
      break;
    }

    stop = 0;
    if (!this_ptr->listener_) {
      break;
    }

    // 通知进度
    Progress progress;
    if (this_ptr->UpdateProgress(dltotal, dlnow, ultotal, ulnow, &progress)) {
      this_ptr->listener_->TaskUpdate(Status::kRunning, &progress, nullptr);
    }

  } while (false);

  return stop;
}

}  // namespace nas
