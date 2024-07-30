/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: panming@ludashi.com
 * @Date: 2023-07-28 16:30:16
 */

#include "base/at_exit.h"
#include "base/command_line.h"
#include "nas/common/main_entry.h"

#if BUILDFLAG(IS_WIN) && !defined(WIN_CONSOLE_APP)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE prev, wchar_t*, int) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(0, nullptr);
#else   // !defined(WIN_CONSOLE_APP)
int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);
#endif  // !defined(WIN_CONSOLE_APP)
  return MainEntry();
}