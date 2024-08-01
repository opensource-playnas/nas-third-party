/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 15:22:38
 */

#ifndef NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_NETWORK_TASK_H_
#define NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_NETWORK_TASK_H_
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <atomic>
#include <memory>
#include <chrono>
#include <thread>
#include "curl/curl.h"
//#include "curl_param.h"

namespace nas {

enum HttpCode
{
  kOk = 200,
  kBadRequest = 400
};

class NetworkTask {
 public:
  // 任务状态
  enum Status {
    kNormal = 0,   // 排队或未开始的状态
    kRunning = 1,  // 正在运行
    kSuccess = 2,  // 成功
    kFail = 3,     // 失败
  };

  // 标识网络任务的类型
  enum Type {
    kUnknown,
    kHttpDownload,
    kHttpGet,
    kHttpPost,
  };

  // 标识网络任务的进度,主要用于通知业务层
  struct Progress {
    long long total_data_length_byte = 0;  // 总大小
    long long recv_data_length_byte = 0;   // 收到的数据大小
    long long time = 0;                    // 已用时间, 毫秒
    double percent = 0;                       // 进度(0~100)
    long long current_speed_byte = 0;      // 瞬时速度 B/s
  };

  struct Result {
    long http_code = 0;
    CURLcode curl_code = CURLE_FAILED_INIT;
    long long request_time = 0;            // 请求时长, 毫秒
    long long avg_current_speed_byte = 0;  // 平均速度 B/s
  };

  class Listener {
   public:
    ~Listener() = default;
    // Progress 只有 在running的时候才会有，Result只有结束时才有
    virtual void TaskUpdate(Status task_status,
                            const Progress* task_progress,
                            const Result* task_result) = 0;
  };

 public:
  NetworkTask();
  virtual ~NetworkTask();

  bool Init(const char* url);
  void SetTaskId(const char* id);
  void AddHeader(const char* key, const char* value);
  bool SetParam(const char* key, const char* value);
  bool SetParam(const char* key, const int& value);

  void SetListener(Listener* listener);
  void Start();
  CURLcode Pause();
  CURLcode Resume();
  virtual bool Cancel(bool is_delete_local_file = false);
  void WaitThreadQuit();
  void Reset();
  Status CurrentTaskStatus() { return status_; }

  bool GetResponseContent(std::string* content);
  virtual Type TaskType() = 0;

 protected:
  // 派生类通过该接口设置请求类型,比如 GET POST或者其他自定义选项
  virtual void SetUrlOpt(CURL* task_curl) = 0;
  // 派生类通过该接口接收数据信息，如果不处理，返回false ，反之 true
  virtual bool WriteFileData(void* data, size_t size, size_t nmemb, size_t * real_write_size) { return false; }
  virtual void BuildData(CURL* task_curl){};

 private:
  virtual void Perform();

  bool UpdateProgress(curl_off_t dltotal,
                      curl_off_t dlnow,
                      curl_off_t ultotal,
                      curl_off_t ulnow,
                      Progress* progress);

protected:
  bool UpdateResult(Result* result);

  //写数据回调
  static size_t WriteFuncCallback(void* data,
                                  size_t size,
                                  size_t nmemb,
                                  void* network_task_ptr);
  //进度回调
  static int ProgressFuncCallback(void* network_task_ptr,
                                  curl_off_t dltotal,
                                  curl_off_t dlnow,
                                  curl_off_t ultotal,
                                  curl_off_t ulnow);

 protected:
  CURL* task_curl_ptr_ = nullptr;
  Status status_ = kNormal;

  std::atomic_bool cancel_ = false;

  //CurlParam curl_param_;
  CURLcode curl_code_ = CURLE_FAILED_INIT;
  Listener* listener_ = nullptr;

  //失败重试次数
  int setparam_retry_count = 3;
  //压缩方式
  int setparam_compress_mode = 0;

  //计算下载速度时使用
  std::chrono::steady_clock::time_point last_recv_time_;
  curl_off_t last_downloaded_bytes_ = 0;

  std::shared_ptr<std::thread> thread_ = nullptr;

  bool first_has_total_value_ = true;
  unsigned long long file_init_size_ = 0;
  unsigned long long file_total_size_ = 0;
  //std::ofstream log_file_;
  std::string task_id_;
  std::string url_;
};
}  // namespace nas

#endif  // NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_NETWORK_TASK_H_