/*
* @Description: 
* @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2022-12-01 14:59:00
*/
#ifndef NAS_SERVICE_MANAGER_TEST_RUN_PROCESS_H
#define NAS_SERVICE_MANAGER_TEST_RUN_PROCESS_H

#include <memory>
#include <vector>

#include "base/command_line.h"
#include "base/files/file.h"
#include "base/process/process.h"
#include "base/process/launch.h"

namespace nas {

class TestRunProcess {
 public:
  // 启动进程, 根据进程的输出信息判断其是否启动成功
  // wait_time: 等待最长时间, 单位 ms
  // test_case: 测试的条件, 可能多个
  // all_case_match: 是否需要所有的条件满足
  // 通过日志文件 读取输出信息
  bool TestRun(bool high_permission, const base::CommandLine& command_line,
               const base::FilePath& current_dir, const uint32_t wait_time,
               const std::vector<std::string>& test_case, bool all_case_match,
               const base::FilePath& log_path);
  bool ProcessValid();
  // 对base::Process的包装
  bool WaitProcessExit(int* exit_code);
  bool TerminateProcess();
  //
  // 通过pid重新设置process
  void ResetProcess(base::ProcessId pid);
  base::ProcessId ProcessId();

  static void ReducePermissions(base::LaunchOptions& options);

 private:
  bool TestCase(const std::string& content,
                const std::vector<std::string>& test_case, bool all_case_match);

 private:
  base::Process process_;
};

}  // namespace nas

#endif  // NAS_SERVICE_MANAGER_REDIRTCT_LAUNCH_H