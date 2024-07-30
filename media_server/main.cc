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
#include "base/task/thread_pool/thread_pool_instance.h"

#include "nas/common/winsock_initializer.h"

#include "nas/common/call_result.h"
#include "nas_config.h"
#include "src/media_server_async_impl.h"
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
  base::ThreadPoolInstance::CreateAndStartWithDefaultParams(
      "media.service_threadd_pool");
  nas::CallResult::error_source = nas::ErrorSource::kMediaService;
  nas::InitLog("media_server.log");
  LOG(INFO) << "---------start----------";

  /*std::string err_table = nas::FormatErrorDescTable(
      nas::CallResult::error_source, nas::kMediaErrorList);*/

  RunServer();
  base::ThreadPoolInstance::Get()->Shutdown();
  LOG(INFO) << "---------end----------";
  return 0;
}

void RunServer() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  std::string self_ip = "127.0.0.1:0";
  if (command_line && command_line->HasSwitch("default")) {
    self_ip = "127.0.0.1:50012";
  }
  nas::MediaServiceAsyncImpl service;
  service.RunServer(self_ip);
}

#if BUILDFLAG(IS_LINUX)

int main2(int argc, const char* argv[]) {
  auto full_cmd =
      "/home/lxh/workspace/dev/base_chromium/src/out/linux_dbg/ffmpeg/ffmpeg "
      "-hide_banner -err_detect ignore_err -analyzeduration 200M   -f "
      "matroska,webm -autorotate 0 -i "
      "/home/lxh/workspace/video/dzm.mkv -map_metadata -1 -map_chapters -1 "
      "-threads 0  -map 0:0 -map 0:1 -map -0:s  -codec:v:0 libx264  -preset "
      "veryfast -crf 23 -maxrate 5850750 -bufsize 11701500   -level 51  "
      "-x264opts:0 "
      "subme=0:me_range=4:rc_lookahead=10:me=dia:no_chroma_me:8x8dct=0:"
      "partitions=none -force_key_frames:0 expr:gte(t,0+n_forced*3)"
      " -sc_threshold:v:0 0  -vf "
      "scale=trunc(min(max(iw\\,ih*a)\\,min(1920\\,1080*a))/"
      "2)*2:trunc(min(max(iw/a\\,ih)\\,min(1920/a\\,1080))/"
      "2)*2,format=yuv420p -color_primaries bt709 -color_trc bt709 "
      "-colorspace bt709  -acodec copy  -copyts -avoid_negative_ts disabled "
      "-max_muxing_queue_size 2048 -f hls -max_delay 5000000 -hls_time 3 "
      "-hls_segment_type mpegts -start_number 00000 -hls_segment_filename "
      "/home/lxh/.local/share/nas/video_server/transcode/abcdefg/slice-%05d.ts "
      "-hls_playlist_type vod -hls_list_size 0 -y "
      "/home/lxh/.local/share/nas/video_server/transcode/abcdefg/index.m3u8";
  LOG(INFO) << "ffmpeg cmd: " << full_cmd;

  std::vector<std::string> cmd_vec = base::SplitString(
      full_cmd, " ", base::KEEP_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  // Create the pipe for the child process's STDOUT.

  do {
    int pipe_fd[2];
    if (pipe(pipe_fd) < 0) {
      LOG(INFO) << "Failed to create pipe";
      break;
    }

    base::LaunchOptions launch_opt;
    launch_opt.fds_to_remap.emplace_back(pipe_fd[1], STDOUT_FILENO);
    launch_opt.fds_to_remap.emplace_back(pipe_fd[1], STDERR_FILENO);

    auto tmp_process = base::LaunchProcess(cmd_vec, launch_opt);
    if (!tmp_process.IsValid()) {
      LOG(INFO) << "run ffmpeg failed";
      break;
    }

    do {
      char buffer[256] = {0};
      ssize_t bytes_read = read(pipe_fd[0], buffer, sizeof(buffer));
      if (bytes_read <= 0) {
        break;
      }

      std::string tmp_str(buffer, bytes_read);
      LOG(INFO) << tmp_str;

    } while (true);

    close(pipe_fd[0]);

    close(pipe_fd[1]);
  } while (false);

  return 0;
}
#endif
