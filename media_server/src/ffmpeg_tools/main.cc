

#include <memory>
#include <string>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/process/launch.h"
#include "base/strings/string_split.h"
#include "base/threading/simple_thread.h"

#include "nas_config.h"


#include "ffmpeg_helper.h"


int main(int argc, const char* argv[]) {
  base::AtExitManager exit_manager;
  base::CommandLine::Init(argc, argv);
  [[maybe_unused]] int unuse = system("chcp 65001");
  nas::InitLog("ffmpeg_tools.log");
  LOG(INFO) << "---------start----------";
  nas::FFmpegHelper::GetInstance()->RunCommand();
  return 0;
}
