/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2022-12-28 17:53:47
 */
#ifndef NAS_COMMON_CRASHPAD_H_
#define NAS_COMMON_CRASHPAD_H_

#include <memory>
#include <string>


namespace crashpad {
class CrashpadClient;
}

namespace nas {
static const char* kDumpUrl = "http://crashrpt.playnas.com/pc/updata/dump";
static const char* kVerKey = "ver";
static const char* kNidKey = "nid";
static const char* kContactKey = "contact";
static const char* kSrcKey = "src";
static const char* kVerifyKey = "verify";
static const char* kPidKey = "pid";
static const char* kOsKey = "os";

#if BUILDFLAG(IS_WIN)
  static const char* kPipeSwitch = "nas_crashpad_pipe_name";
  static const char* kPipeArg = " --nas_crashpad_pipe_name=";
#elif BUILDFLAG(IS_LINUX)
  static const char* kSockSwitch = "nas_crashpad_sock";
  static const char* kPidSwitch = "nas_crashpad_pid";
  static const char* kSockArg = " --nas_crashpad_sock=";
  static const char* kPidArg = " --nas_crashpad_pid=";
#endif

class Crashpad {
 public:
#if BUILDFLAG(IS_WIN)
  //! \brief The interface for an application to have Crashpad monitor
  //!     it for crashes
  //! \param[in] handler_path is crashpad_handler path
  //! root directory level. \param[out] ipc_pipe The full name of the crash
  //! handler IPC pipe. This is
  //!     a string of the form `&quot;\\.\pipe\NAME&quot;`.
  //!
  //! \return `true` on success, `false` on failure.
  static bool InitializeCrashpad(const base::FilePath& handler_path, std::wstring* pipe_name);

  static bool SetHandlerIPCPipeByCmd();

  //! \brief The child process need to use the interface
  //!
  //! \return `true` on success, `false` on failure.
  static bool SetHandlerIPCPipe();


#elif BUILDFLAG(IS_LINUX)
  //! \brief The interface for an application to have Crashpad monitor
  //!     it for crashes
  //! \param[in] The level value is the module relative to the crashpad_handler
  //! root directory level. \param[out] sock The socket connected to the
  //! handler, if not `nullptr`. \param[out] pid The handler's process ID, if
  //! not `nullptr`.
  //!
  //! \return `true` on success, `false` on failure.
  static bool InitializeCrashpad(const base::FilePath& handler_path, int* sock, pid_t* pid);

  static bool SetHandlerSocketByCmd();

  //! \brief The child process need to use the interface
  //!
  //! \return `true` on success, `false` on failure.
  static bool SetHandlerSocket();
#endif
  //! \brief The child process need to use the interface
  //!
  //! \return `true` on success, `false` on failure.
  static bool SetHandlerIPC();

 private:
  static std::shared_ptr<crashpad::CrashpadClient> GetCrashpadClient(
      const base::FilePath& handler_path);
  static base::FilePath GetCrashpadHandlerPath(unsigned short level);

};  // class crashpad

}  // namespace nas

#endif  // NAS_COMMON_CRASHPAD_H_
