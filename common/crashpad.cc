/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2022-12-28 17:53:54
 */

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/hash/md5.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/system/sys_info.h"

#include "third_party/crashpad/crashpad/client/crash_report_database.h"
#include "third_party/crashpad/crashpad/client/crashpad_client.h"
#include "third_party/crashpad/crashpad/client/settings.h"

#include "crashpad.h"
#include "nas/common/device_config.h"
#include "nas/common/nas_config.h"
#include "nas/common/service_manager/services_run_info.h"

namespace nas {
#if BUILDFLAG(IS_WIN)
bool Crashpad::InitializeCrashpad(const base::FilePath& handler_path,
                                  std::wstring* pipe_name) {
  std::shared_ptr<crashpad::CrashpadClient> client =
      GetCrashpadClient(handler_path);
  if (!client) {
    return false;
  }

  if (pipe_name) {
    *pipe_name = client->GetHandlerIPCPipe();
  }

  return true;
}

bool Crashpad::SetHandlerIPCPipeByCmd() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (!command_line) {
    LOG(ERROR) << __func__ << " command line is null!";
    return false;
  }

  base::FilePath pipe = command_line->GetSwitchValuePath(kPipeSwitch);
  std::string name = pipe.AsUTF8Unsafe();
  LOG(INFO) << __func__ << ": pipe_name = " << name;
  if (name.empty()) {
    return false;
  }

  std::shared_ptr<crashpad::CrashpadClient> client =
      std::make_shared<crashpad::CrashpadClient>();
  return client->SetHandlerIPCPipe(base::UTF8ToWide(name));
}

bool Crashpad::SetHandlerIPCPipe() {
  ServiceRunInfo run_info;
  run_info.InitRunInfo();
  std::string name = run_info.GetServiceMgrCrashpadPipeName();
  LOG(INFO) << "Crashpad::SetHandlerIPCPipe name: " << name;
  if (name.empty()) {
    return false;
  }
  std::shared_ptr<crashpad::CrashpadClient> client =
      std::make_shared<crashpad::CrashpadClient>();
  return client->SetHandlerIPCPipe(base::UTF8ToWide(name));
}
#elif BUILDFLAG(IS_LINUX)
bool Crashpad::InitializeCrashpad(const base::FilePath& handler_path,
                                  int* sock,
                                  pid_t* pid) {
  std::shared_ptr<crashpad::CrashpadClient> client = GetCrashpadClient(handler_path);
  if (!client) {
    return false;
  }

  if (sock && pid) {
    client->GetHandlerSocket(sock, pid);
  }

  return true;
}

bool Crashpad::SetHandlerSocketByCmd() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (!command_line) {
    LOG(ERROR) << __func__ << " command line is null!";
    return false;
  }

  int sock = 0;
  pid_t pid = 0;
  std::string sock_str =
      command_line->GetSwitchValuePath(kSockSwitch).AsUTF8Unsafe();
  std::string pid_str =
      command_line->GetSwitchValuePath(kPidSwitch).AsUTF8Unsafe();
  base::StringToInt(sock_str, &sock);
  base::StringToInt(pid_str, &pid);
  LOG(INFO) << __func__ << ": sock = " << sock << ", pid = " << pid;
  if (sock == 0 && pid == 0) {
    return false;
  }

  crashpad::ScopedFileHandle socket;
  socket.reset(sock);
  std::shared_ptr<crashpad::CrashpadClient> client =
      std::make_shared<crashpad::CrashpadClient>();
  return client->SetHandlerSocket(std::move(socket), pid);
}

bool Crashpad::SetHandlerSocket() {
  ServiceRunInfo run_info;
  run_info.InitRunInfo();
  int sock = 0;
  int pid = 0;
  run_info.GetServiceMgrCrashpadSockPid(sock, pid);
  LOG(INFO) << __func__ << ": sock = " << sock << ", pid = " << pid;

  crashpad::ScopedFileHandle socket;
  socket.reset(sock);
  std::shared_ptr<crashpad::CrashpadClient> client =
      std::make_shared<crashpad::CrashpadClient>();
  return client->SetHandlerSocket(std::move(socket), pid);
}
#endif

bool Crashpad::SetHandlerIPC() {
#if BUILDFLAG(IS_WIN)
  return SetHandlerIPCPipe();
#elif BUILDFLAG(IS_LINUX)
  // TODO
  return SetHandlerSocket();
#endif
}

std::shared_ptr<crashpad::CrashpadClient> Crashpad::GetCrashpadClient(
    const base::FilePath& handler_path) {
  std::shared_ptr<crashpad::CrashpadClient> client = nullptr;
  do {
    // Directory where reports will be saved. Important! Must be writable or
    // crashpad_handler will crash.
    base::FilePath reports_dir = nas::GetNasDumpDir();

    // Directory where metrics will be saved. Important! Must be writable or
    // crashpad_handler will crash.
    base::FilePath metrics_dir = reports_dir;

    // Metadata that will be posted to the server with the crash report map
    std::map<std::string, std::string> annotations;
    // 由于需要崩溃进程的ver，这里拿不到，所以会在crashpad_handler重新赋值
    annotations[kVerKey] = "";  
    annotations[kNidKey] = nas::device::GetNid(0);
    annotations[kContactKey] = "contact";
    annotations[kSrcKey] = "";  // crashpad_handler 里面会重新赋值
    // verify由src、nid、ver、nas_dump_uper组成的字符串取md5，由crashpad_handler去
    // 拼装
    annotations[kVerifyKey] = "";
    annotations[kPidKey] = "nas_pid";
    annotations[kOsKey] = base::SysInfo::OperatingSystemName() + " " +
                          base::SysInfo::OperatingSystemVersion() + " " +
                          base::SysInfo::OperatingSystemArchitecture();

    // Disable crashpad rate limiting so that all crashes have dmp files
    
    std::vector<std::string> arguments;
    arguments.push_back("--no-rate-limit");
    arguments.push_back("--no-identify-client-via-url");

    // Initialize Crashpad database
    std::unique_ptr<crashpad::CrashReportDatabase> database =
        crashpad::CrashReportDatabase::Initialize(reports_dir);
    if (database == NULL)
      break;

    // Enable automated crash uploads
    crashpad::Settings* settings = database->GetSettings();
    if (settings == NULL)
      break;
    settings->SetUploadsEnabled(true);

    // File paths of attachments to uploaded with minidump file at crash time
    std::vector<base::FilePath> attachments;
    base::FilePath attachment(
        reports_dir.Append(base::FilePath::FromUTF8Unsafe("attachment.txt")));
    attachments.push_back(attachment);

    // Start crash handler
    client = std::make_shared<crashpad::CrashpadClient>();
    bool status =
        client->StartHandler(handler_path, reports_dir, metrics_dir, kDumpUrl,
                             annotations, arguments, true, false, attachments);
    if (!status) {
      LOG(ERROR) << __func__ << " start handler status in false!";
      client = nullptr;
      break;
    }
  } while (0);

  return client;
}

base::FilePath Crashpad::GetCrashpadHandlerPath(unsigned short level) {
  DCHECK(level < 255);
  base::FilePath handler_path;
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (!command_line) {
    LOG(ERROR) << __func__ << " command line is null!";
    return handler_path;
  }

  handler_path = command_line->GetProgram().DirName();
  // 0 是当前目录层级
  while (level--) {
    handler_path = handler_path.DirName();
  }

#if BUILDFLAG(IS_WIN)
  handler_path = handler_path.Append(
      base::FilePath::FromUTF8Unsafe("crashpad_handler.exe"));
#elif BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
  handler_path =
      handler_path.Append(base::FilePath::FromUTF8Unsafe("crashpad_handler"));
#endif
  LOG(INFO) << "crashpad_handler path is :" << handler_path;
  return handler_path;
}

}  // namespace nas