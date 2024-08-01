/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-06 19:51:41
 */

#include <iostream>
#include <memory>
#include <string>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/threading/simple_thread.h"

#include "nas/common/call_result.h"
#include "nas/common/main_entry.h"
#include "nas/common/nas_config.h"
#include "nas/common/winsock_initializer.h"


#include "download_grpc_server.h"

void RunServer() {
  nas::DownloadServiceImpl service;
  service.RunServer();
}

 int MainEntry(){
#ifdef _WIN32
  winsock_initializer winsock_init;
#else
  grpc::internal::GrpcLibraryInitializer init_grpc;
#endif /* _WIN32 */
   [[maybe_unused]] int unuse = system("chcp 65001");
   nas::InitLog("download_server.log");
   LOG(INFO) << "---------start----------";
   nas::CallResult::error_source = nas::ErrorSource::kDownloadService;
   base::ThreadPoolInstance::Create("download_service");
   base::ThreadPoolInstance::Get()->StartWithDefaultParams();

   RunServer();

   LOG(INFO) << "---------end----------";
   return 0;
 }