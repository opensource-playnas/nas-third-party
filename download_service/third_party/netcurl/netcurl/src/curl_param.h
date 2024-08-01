/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-03-08 16:13:31
*/

#ifndef NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_GLOBAL_CURL_PARAM_H_
#define NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_GLOBAL_CURL_PARAM_H_

#include <curl/curl.h>

#include <string>

class CurlParam {
 public:
  CurlParam();
  ~CurlParam();

  CurlParam(const CurlParam&) = delete;
  CurlParam& operator=(const CurlParam&) = delete;

	// 提供一些默认的设置
  void Init(CURL* curl_ptr);

  bool SetParam(const char* name, const char* value);
  bool SetParam(const char* name, const int &value);

 private:
  CURL* task_curl_ptr_;
};

#endif  // NAS_INSTALLER_UNINSTALLER_SRC_NETWORK_GLOBAL_CURL_PARAM_H_