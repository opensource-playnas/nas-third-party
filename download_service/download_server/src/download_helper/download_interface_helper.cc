/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 16:34:21
 */
#include "download_interface_helper.h"
#include "base/path_service.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "nas/common/path/file_path_unit.h"

#include "utils/utils.h"

namespace nas {

nas::DownloadInterfaceHelper* DownloadInterfaceHelper::GetInstance() {
  return base::Singleton<DownloadInterfaceHelper>::get();
}

bool DownloadInterfaceHelper::Init() {
  bool ret = false;

  do {
    base::FilePath exe_dir;
    base::PathService::Get(base::DIR_EXE, &exe_dir);
    exe_path_ = path_unit::BaseFilePathToU8(exe_dir);
    dir_path_ = exe_dir.Append(FILE_PATH_LITERAL("download"));

    LOG(INFO) << "dir_path_ : " << dir_path_;

    if (!base::DirectoryExists(dir_path_)) {
      break;
    }

    if (!InitNetCurl()) {
      LOG(ERROR) << "init netcurl failed!";
    }

    if (!InitNetBt()) {
      LOG(ERROR) << "init netbt failed!";
    }

    if (!InitNetMule()) {
      LOG(ERROR) << "init netmule failed!";
    }

  } while (0);

  return ret;
}

bool DownloadInterfaceHelper::UnInit() {
  if (pfn_curl_uninit_) {
    pfn_curl_uninit_();
  }

  if (pfn_mule_uninit_) {
    pfn_mule_uninit_();
  }

  if (pfn_bt_uninit_) {
    pfn_bt_uninit_();
  }
  return true;
}

CurlDownloadTask* DownloadInterfaceHelper::CreateCurlDownloadTask() {
  if (pfn_create_curl_download_task_) {
    return pfn_create_curl_download_task_();
  }
  return nullptr;
}

TorrentFileDownloadTask*
DownloadInterfaceHelper::CreateTorrentFileDownloadTask() {
  if (pfn_create_torrent_file_download_task_) {
    return pfn_create_torrent_file_download_task_();
  }
  return nullptr;
}

MagnetDownloadTask* DownloadInterfaceHelper::CreateMagnetDownloadTask() {
  if (pfn_create_magnet_download_task_) {
    return pfn_create_magnet_download_task_();
  }
  return nullptr;
}

MuleDownloadTask* DownloadInterfaceHelper::CreateMuleDownloadTask() {
  if (pfn_create_mule_download_task_) {
    return pfn_create_mule_download_task_();
  }
  return nullptr;
}

void DownloadInterfaceHelper::ReleaseCurlTask(DownloadTaskInterface* task) {
  if (pfn_release_curl_task_ && task) {
    pfn_release_curl_task_(task);
  }
}

void DownloadInterfaceHelper::ReleaseTorrentTask(DownloadTaskInterface* task) {
  if (pfn_release_bt_task_ && task) {
    pfn_release_bt_task_(task);
  }
}

void DownloadInterfaceHelper::ReleaseMuleTask(DownloadTaskInterface* task) {
  if (pfn_release_mule_task_ && task) {
    pfn_release_mule_task_(task);
  }
}

void DownloadInterfaceHelper::WaitInitComplete() {
  if (pfn_wait_init_bt_finish_) {
    pfn_wait_init_bt_finish_();
  }

  if (pfn_wait_init_mule_finish_) {
    pfn_wait_init_mule_finish_();
  }
}

BtSettingInterface* DownloadInterfaceHelper::GetBtSettingsInterface() {
  DCHECK(pfn_bt_settings_);
  if (pfn_bt_settings_) {
    return pfn_bt_settings_();
  }
  return NULL;
}

AmuleSettingInterface* DownloadInterfaceHelper::GetAmuleSettingsInterface() {
  DCHECK(pfn_amule_settings_);
  if (pfn_amule_settings_) {
    return pfn_amule_settings_();
  }
  return NULL;
}

DownloadInterfaceHelper::DownloadInterfaceHelper() {}

DownloadInterfaceHelper::~DownloadInterfaceHelper() {}

bool DownloadInterfaceHelper::InitNetCurl() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("netcurl.dll");
#else
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("libnetcurl.so");
#endif
  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitNetCurl failed";
    return false;
  }

  GetFunctionByDll(&pfn_create_curl_download_task_, lib_dll, "CreateTask");
  if (!pfn_create_curl_download_task_) {
    LOG(ERROR) << "get pfn_create_curl_download_task func failed";
    return false;
  }

  GetFunctionByDll(&pfn_release_curl_task_, lib_dll, "ReleaseTask");
  if (!pfn_release_curl_task_) {
    LOG(ERROR) << "get pfn_release_curl_task_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_curl_init_, lib_dll, "Init");
  if (!pfn_curl_init_) {
    LOG(ERROR) << "get pfn_curl_init_ func failed";
    return false;
  }
  pfn_curl_init_();

  GetFunctionByDll(&pfn_curl_uninit_, lib_dll, "UnInit");
  if (!pfn_curl_uninit_) {
    LOG(ERROR) << "get pfn_curl_uninit_ func failed";
    return false;
  }

  is_curl_dll_loaded_ = true;
  return true;
}

bool DownloadInterfaceHelper::InitNetBt() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("netbt.dll");
#else
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("libnetbt.so");
#endif
  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitNetBt failed";
    return false;
  }

  GetFunctionByDll(&pfn_init_, lib_dll, "Init");
  if (!pfn_init_) {
    LOG(ERROR) << "get pfn_Init_bt func failed";
    return false;
  }
  std::string profile_path = "--profile=";
  profile_path += path_unit::BaseFilePathToU8(utils::GetDownloadConfigDir());
  char* argv[2];
  argv[0] = const_cast<char*>(exe_path_.c_str());
  argv[1] = const_cast<char*>(profile_path.c_str());
  pfn_init_(2, argv);

  GetFunctionByDll(&pfn_wait_init_bt_finish_, lib_dll, "WaitInitFinish");
  if (!pfn_wait_init_bt_finish_) {
    LOG(ERROR) << "get pfn_wait_init_finish_ func failed";
    return false;
  }
  pfn_wait_init_bt_finish_();

  GetFunctionByDll(&pfn_create_torrent_file_download_task_, lib_dll,
                   "CreateTorrentFileTask");
  if (!pfn_create_torrent_file_download_task_) {
    LOG(ERROR) << "get pfn_create_torrent_file_download_task_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_create_magnet_download_task_, lib_dll,
                   "CreateMagnetTask");
  if (!pfn_create_magnet_download_task_) {
    LOG(ERROR) << "get pfn_create_magnet_download_task_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_release_bt_task_, lib_dll, "ReleaseTask");
  if (!pfn_release_bt_task_) {
    LOG(ERROR) << "get pfn_release_bt_task_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_bt_settings_, lib_dll, "CreateBtSettings");
  if (!pfn_bt_settings_) {
    LOG(ERROR) << "get pfn_bt_settings_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_bt_uninit_, lib_dll, "UnInit");
  if (!pfn_bt_uninit_) {
    LOG(ERROR) << "get pfn_bt_uninit_ func failed";
    return false;
  }

  is_bt_dll_loaded_ = true;
  return true;
}

bool DownloadInterfaceHelper::InitNetMule() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("netmule.dll");
#else
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("libnetmule.so");
#endif
  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitNetMule failed";
    return false;
  }

  GetFunctionByDll(&pfn_init_, lib_dll, "Init");
  if (!pfn_init_) {
    LOG(ERROR) << "get pfn_Init_mule func failed";
    return false;
  }

  std::string profile_path = "--config-dir=";
  profile_path += path_unit::BaseFilePathToU8(
      utils::GetDownloadConfigDir().Append(FILE_PATH_LITERAL("nasmule")));
  char* argv[2];
  argv[0] = const_cast<char*>(exe_path_.c_str());
#if BUILDFLAG(IS_WIN)
  std::wstring wstr = base::UTF8ToWide(exe_path_);
  std::string multi_byte_string = base::SysWideToNativeMB(wstr);
  argv[0] = const_cast<char*>(multi_byte_string.c_str());
#endif
  argv[1] = const_cast<char*>(profile_path.c_str());
  pfn_init_(2, argv);

  GetFunctionByDll(&pfn_wait_init_mule_finish_, lib_dll, "WaitInitFinish");
  if (!pfn_wait_init_mule_finish_) {
    LOG(ERROR) << "get pfn_wait_init_finish_ func failed";
    return false;
  }
  pfn_wait_init_mule_finish_();

  GetFunctionByDll(&pfn_create_mule_download_task_, lib_dll, "CreateTask");
  if (!pfn_create_mule_download_task_) {
    LOG(ERROR) << "get pfn_create_mule_download_task_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_release_mule_task_, lib_dll, "ReleaseTask");
  if (!pfn_release_mule_task_) {
    LOG(ERROR) << "get pfn_release_mule_task_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_amule_settings_, lib_dll, "CreateAuleSettings");
  if (!pfn_amule_settings_) {
    LOG(ERROR) << "get pfn_amule_settings_ func failed";
    return false;
  }

  GetFunctionByDll(&pfn_mule_uninit_, lib_dll, "UnInit");
  if (!pfn_mule_uninit_) {
    LOG(ERROR) << "get pfn_mule_uninit_ func failed";
    return false;
  }

  is_mule_dll_loaded_ = true;
  return true;
}

base::NativeLibrary DownloadInterfaceHelper::GetDllModuleHandle(
    base::FilePath::StringPieceType dll_name) {
  auto dll_path = dir_path_.Append(dll_name);
  base::NativeLibraryLoadError error;
  base::NativeLibrary lib_dll = base::LoadNativeLibrary(dll_path, &error);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary  failed, path: " << dll_path.value()
               << "error: " << error.ToString();
    return nullptr;
  }
  return lib_dll;
}

template <class T>
void DownloadInterfaceHelper::GetFunctionByDll(T* func_ptr,
                                               base::NativeLibrary dll_handle,
                                               const char* func_name) {
  if (!func_ptr) {
    return;
  }
  *func_ptr =
      (T)base::GetFunctionPointerFromNativeLibrary(dll_handle, func_name);
}

bool DownloadInterfaceHelper::IsLoadedNetCurl() {
  return is_curl_dll_loaded_;
}

bool DownloadInterfaceHelper::IsLoadedNetMule() {
  return is_mule_dll_loaded_;
}

bool DownloadInterfaceHelper::IsLoadedNetBt() {
  return is_bt_dll_loaded_;
}

}  // namespace nas
