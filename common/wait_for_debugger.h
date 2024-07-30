#ifndef NAS_COMMON_WAIT_FOR_DEBUGGER_
#define NAS_COMMON_WAIT_FOR_DEBUGGER_

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "base/debug/stack_trace.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"

namespace nas {
void WaitForDebuggerIfNecessary() {
  const base::CommandLine* command_line =
      base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch("wait-for-debugger")) {
    base::FilePath exe_path =
        command_line->GetProgram().BaseName().RemoveExtension();
#if defined(OS_WIN)
    std::wstring app = exe_path.value();
    std::wstring message =
        base::StringPrintf(L"%s - %ld", app.c_str(), GetCurrentProcessId());
    MessageBoxW(NULL, message.c_str(), app.c_str(), MB_OK | MB_SETFOREGROUND);
#else
    LOG(ERROR) << app << " waiting for GDB. pid: " << getpid();
    base::debug::WaitForDebugger(60, true);
#endif
  }
}
}  // namespace nas

#endif