// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/02/14 15:10

#include <memory>
#include <string>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/process/launch.h"
#include "base/strings/string_split.h"
#include "base/threading/simple_thread.h"

#include "nas/common/call_result.h"
#include "nas/common/winsock_initializer.h"

#include "nas_config.h"
#include "src/dlna_server_async_impl.h"
#include "src/public_define.h"

#if BUILDFLAG(IS_WIN)
#pragma comment( \
    linker,      \
    "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")  // 设置入口地址
#endif

void RunServer();

int main(int argc, const char* argv[]) {
#ifdef _WIN32
  winsock_initializer winsock_init;
#else
  grpc::internal::GrpcLibraryInitializer init_grpc;
#endif /* _WIN32 */
  base::AtExitManager exit_manager;
  base::CommandLine::Init(argc, argv);
  nas::CallResult::error_source = nas::ErrorSource::kDlnaService;
  [[maybe_unused]] int unuse = system("chcp 65001");
  nas::InitLog("dlna_server.log");
  LOG(INFO) << "---------start----------";
  std::string err_table = nas::FormatErrorDescTable(
      nas::CallResult::error_source, nas::kDlnaErrorList);

  RunServer();
  LOG(INFO) << "---------end----------";
  return 0;
}

void RunServer() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  std::string self_ip = "127.0.0.1:0";
  if (command_line && command_line->HasSwitch("default")) {
    self_ip = "127.0.0.1:50013";
  }
  nas::DlnaServiceAsyncImpl service;
  service.RunServer(self_ip);
}
