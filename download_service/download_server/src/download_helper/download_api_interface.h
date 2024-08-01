/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-03-07 16:33:06
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_API_INTERFACE_H_
#define NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_API_INTERFACE_H_

// 三个dll头文件
#include "nas/download_service/third_party/netcurl/netcurl/src/curl_task_public_define.h"
#include "nas/download_service/third_party/amule/amule/src/mule_task_public_define.h"
#include "nas/download_service/third_party/qBittorrent/qBittorrent/src/app/torrent_task_public_define.h"

#if BUILDFLAG(IS_WIN)
#define DECL_NAME __cdecl
#else
#define DECL_NAME __attribute__((__cdecl__))
#endif

typedef CurlDownloadTask*(DECL_NAME* PfnCreateCurlDownloadTask)();
typedef TorrentFileDownloadTask*(
    DECL_NAME* PfnCreateTorrentFileDownloadTask)();
typedef MagnetDownloadTask*(DECL_NAME* PfnCreateMagnetDownloadTask)();
typedef MuleDownloadTask*(DECL_NAME* PfnCreateMuleDownloadTask)();
typedef int(DECL_NAME* PfnInit)(int argc, char* argv[]);
typedef int(DECL_NAME* PfnWaitInitFinish)();
typedef void(DECL_NAME* PfnReleaseTask)(DownloadTaskInterface* task);
typedef BtSettingInterface*(DECL_NAME* PfnBtSettings)();
typedef AmuleSettingInterface*(DECL_NAME* PfnAMuleSettings)();
typedef int(DECL_NAME* PfnCurlInit)();
typedef int(DECL_NAME* PfnUnInit)();

#endif  // NAS_DOWNLOAD_SERVER_SRC_DOWNLOAD_API_INTERFACE_H_