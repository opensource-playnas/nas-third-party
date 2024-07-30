/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2022-12-01 14:59:10
 */
#include "test_run_process.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/process/launch.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#else
#include <signal.h>
#include <errno.h>
#endif

namespace nas {

void TestRunProcess::ReducePermissions(base::LaunchOptions& options) {
#if BUILDFLAG(IS_WIN)
  HANDLE token = NULL;
  BOOL result =
      ::LogonUser(L"NETWORK SERVICE", L"NT AUTHORITY", NULL,
                  LOGON32_LOGON_SERVICE, LOGON32_PROVIDER_DEFAULT, &token);

  auto err = GetLastError();
  LOG(INFO) << "bResult: " << result << ", htoken: " << token
            << ", err: " << err;

  if (token) {
    options.as_user = token;
  }
#elif BUILDFLAG(IS_POSIX)
  options.environment["USER_ID"] = std::to_string(getuid());
#endif
}

static uint32_t LastErr() {
#if BUILDFLAG(IS_WIN)
  return GetLastError();
#elif BUILDFLAG(IS_POSIX)
  return errno;
#endif
}

bool TestRunProcess::TestRun(bool high_permission,
                             const base::CommandLine& command_line,
                             const base::FilePath& current_dir,
                             const uint32_t wait_time,
                             const std::vector<std::string>& test_case,
                             bool all_case_match,
                             const base::FilePath& log_path) {
  base::LaunchOptions options;
  options.current_directory = current_dir;
  if (!high_permission) {
    ReducePermissions(options);
  }

#if BUILDFLAG(IS_WIN)
  // 全部创建来隐藏窗口了
  options.start_hidden = true;
#endif

  process_ = base::LaunchProcess(command_line, options);
  LOG(INFO) << __func__ << ":err = " << LastErr();

  bool run_suc = false;
  const base::TimeTicks kStartTimeTicks = base::TimeTicks::Now();
  do {
    base::PlatformThread::Sleep(base::Milliseconds(1));
    if (base::PathExists(log_path)) {
      base::File log_file(log_path,
                          base::File::FLAG_OPEN | base::File::FLAG_READ);
      const auto kLastErr = LastErr();
      if (!log_file.IsValid()) {
        LOG(WARNING) << __func__ << ": open file err = " << kLastErr;
        continue;
      }
      int64_t file_size = log_file.GetLength();
      if (char* read_buffer = new char[file_size + 1]{0}) {
        log_file.Read(0, read_buffer, file_size);
        log_file.Close();
        {
        std::string temp_buf(read_buffer);
        run_suc = TestCase(temp_buf, test_case, all_case_match);

        }
        delete[] read_buffer;
        read_buffer = nullptr;
      }
      if (run_suc) break;
    }
  } while ((base::TimeTicks::Now() - kStartTimeTicks).InMilliseconds() <
           wait_time);
  return run_suc;
}

// TODO posix下应该通过 pid 判断进程是否存在
bool TestRunProcess::ProcessValid() {
  if (process_.IsValid()) {
#if BUILDFLAG(IS_WIN)
    if (process_.IsRunning()) {
      return true;
    }
#else
    if (kill(ProcessId(), 0) == -1) {
      return false;
    }
    return true;
#endif
  }
  return false;
}

bool TestRunProcess::WaitProcessExit(int* exit_code) {
  return process_.WaitForExit(exit_code);
}

bool TestRunProcess::TerminateProcess() {
  return process_.Terminate(0, true);
}

bool TestRunProcess::TestCase(const std::string& content,
                              const std::vector<std::string>& test_case,
                              bool all_case_match) {
  bool all_match = true;
  bool exist_match = false;
  for (const auto& tcase : test_case) {
    if (content.find(tcase) == std::string::npos) {
      all_match = false;
    } else {
      exist_match = true;
    }
  }
  return all_match || (exist_match && !all_case_match);
}

void TestRunProcess::ResetProcess(base::ProcessId pid) {
  process_ = base::Process::Open(pid);
}

base::ProcessId TestRunProcess::ProcessId() { return process_.Pid(); }

}  // namespace nas