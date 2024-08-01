/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 16:33:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_HELPER_DOWNLOAD_INTERFACE_HELPER_H_
#define NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_HELPER_DOWNLOAD_INTERFACE_HELPER_H_

#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/native_library.h"
#include "nas/download_service/download_server/src/download_helper/download_api_interface.h"

namespace nas {
class DownloadInterfaceHelper {
 public:
  static DownloadInterfaceHelper* GetInstance();
  DownloadInterfaceHelper(const DownloadInterfaceHelper&) = delete;
  DownloadInterfaceHelper& operator=(const DownloadInterfaceHelper&) = delete;

  bool Init();
  bool UnInit();
  CurlDownloadTask* CreateCurlDownloadTask();
  TorrentFileDownloadTask* CreateTorrentFileDownloadTask();
  MagnetDownloadTask* CreateMagnetDownloadTask();
  MuleDownloadTask* CreateMuleDownloadTask();
  void ReleaseCurlTask(DownloadTaskInterface* task);
  void ReleaseTorrentTask(DownloadTaskInterface* task);
  void ReleaseMuleTask(DownloadTaskInterface* task);
  void WaitInitComplete();
  BtSettingInterface* GetBtSettingsInterface();
  AmuleSettingInterface* GetAmuleSettingsInterface();
  bool IsLoadedNetCurl();
  bool IsLoadedNetMule();
  bool IsLoadedNetBt();

 private:
  // This object is a singleton:
  DownloadInterfaceHelper();
  ~DownloadInterfaceHelper();
  friend struct base::DefaultSingletonTraits<DownloadInterfaceHelper>;

  bool InitNetCurl();
  bool InitNetBt();
  bool InitNetMule();

  base::NativeLibrary GetDllModuleHandle(
      base::FilePath::StringPieceType dll_name);

  template <class T>
  void GetFunctionByDll(T* func_ptr,
                        base::NativeLibrary dll_handle,
                        const char* func_name);

 private:
  PfnCreateCurlDownloadTask pfn_create_curl_download_task_ = nullptr;
  PfnCreateTorrentFileDownloadTask pfn_create_torrent_file_download_task_ =
      nullptr;
  PfnCreateMagnetDownloadTask pfn_create_magnet_download_task_ = nullptr;
  PfnCreateMuleDownloadTask pfn_create_mule_download_task_ = nullptr;
  PfnInit pfn_init_ = nullptr;
  PfnWaitInitFinish pfn_wait_init_bt_finish_ = nullptr;
  PfnWaitInitFinish pfn_wait_init_mule_finish_ = nullptr;
  PfnReleaseTask pfn_release_curl_task_ = nullptr;
  PfnReleaseTask pfn_release_bt_task_ = nullptr;
  PfnReleaseTask pfn_release_mule_task_ = nullptr;
  PfnBtSettings pfn_bt_settings_ = nullptr;
  PfnAMuleSettings pfn_amule_settings_ = nullptr;
  base::FilePath dir_path_;
  std::string exe_path_;
  PfnUnInit pfn_mule_uninit_ = nullptr;
  bool is_curl_dll_loaded_ = false;
  bool is_mule_dll_loaded_ = false;
  bool is_bt_dll_loaded_ = false;
  PfnCurlInit pfn_curl_init_ = nullptr;
  PfnUnInit pfn_curl_uninit_ = nullptr;
  PfnUnInit pfn_bt_uninit_ = nullptr;
};  // NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_HELPER_DOWNLOAD_INTERFACE_HELPER_H_
}  // namespace nas

#endif