/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:52:38
 */

#ifndef NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_TASK_INTERFACE_H_
#define NAS_THIRD_PARTY_DOWNLOAD_NETCURL_NETCURL_SRC_CURL_TASK_INTERFACE_H_

#include "curl_export_define.h"

extern "C" EXPORT CurlDownloadTask* CreateTask() {
  CurlDownloadTask* task = new CurlDownloadTaskImpl();
  return task;
}

extern "C" EXPORT void ReleaseTask(DownloadTaskInterface * task) {
  if (task) {
    delete task;
    task = NULL;
  }
}

extern "C" EXPORT int Init() {
  return curl_global_init(CURL_GLOBAL_ALL);
}

extern "C" EXPORT int UnInit() {
  curl_global_cleanup();
  return 0;
}

#endif