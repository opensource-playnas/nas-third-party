
#include "curl_param.h"

#include <map>
static std::map<std::string, CURLoption> g_setparam_map = {
    // ssl相关设置
    {"ssl_verifypeer", CURLOPT_SSL_VERIFYPEER},
    {"ssl_verifyhost", CURLOPT_SSL_VERIFYHOST},
    {"ssl_cainfo", CURLOPT_CAINFO},
    {"ssl_cert_file", CURLOPT_SSLCERT},
    {"ssl_cert_type", CURLOPT_SSLCERTTYPE},
    {"ssl_private_key", CURLOPT_SSLKEY},
    {"ssl_private_keytype", CURLOPT_SSLKEYTYPE},
    {"ssl_private_key_password", CURLOPT_KEYPASSWD},
    // http代理设置
    {"proxy_type", CURLOPT_PROXYTYPE},
    {"proxy_proxy", CURLOPT_PROXY},
    // 是否重定向(0不重定向,1重定向)
    {"follow_location", CURLOPT_FOLLOWLOCATION},
    // 超时
    {"connect_timeout", CURLOPT_CONNECTTIMEOUT},
    {"data_timeout", CURLOPT_TIMEOUT},
    {"encoding", CURLOPT_ENCODING},  // 可以指定压缩gzip等
    {"range", CURLOPT_RANGE},  // 断点下载， 例如：curl_easy_setopt(curl,
                               // CURLOPT_RANGE, "100-999");
};

CurlParam::CurlParam() : task_curl_ptr_(NULL) {}

CurlParam::~CurlParam() {}

void CurlParam::Init(CURL* curl_ptr) {
  task_curl_ptr_ = curl_ptr;
  if (!task_curl_ptr_) {
    return;
  }

  // 忽略SSL
  curl_easy_setopt(task_curl_ptr_, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(task_curl_ptr_, CURLOPT_SSL_VERIFYHOST, 0);
  // 支持重定向
  curl_easy_setopt(task_curl_ptr_, CURLOPT_FOLLOWLOCATION, 1);

  // 超时，单位秒
  // 成功连接服务器前等待多久（连接成功之后就会开始缓冲输出），这个参数是为了应对目标服务器的过载，下线，或者崩溃等可能状况
  // curl_easy_setopt(task_curl_ptr_, CURLOPT_CONNECTTIMEOUT, 60);
  // 从服务器接收缓冲完成前需要等待多长时间。如果目标是个巨大的文件，生成内容速度过慢或者链路速度过慢，这个参数就会很有用
  // curl_easy_setopt(task_curl_ptr_, CURLOPT_TIMEOUT, 30);
}

bool CurlParam::SetParam(const char* name, const char* value) {
  auto iter = g_setparam_map.find(name);
  if (iter != g_setparam_map.end()) {
    curl_easy_setopt(task_curl_ptr_, iter->second, value);
    return true;
  }

  return false;
}

bool CurlParam::SetParam(const char* name, const int& value) {
  auto iter = g_setparam_map.find(name);
  if (iter != g_setparam_map.end()) {
    curl_easy_setopt(task_curl_ptr_, iter->second, value);
    return true;
  }

  return false;
}