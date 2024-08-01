/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-05-15 11:17:02
*/

#ifndef NAS_DOWNLOAD_SERVER_SRC_MEDIA_ERROR_DESC_H_
#define NAS_DOWNLOAD_SERVER_SRC_MEDIA_ERROR_DESC_H_

#include <memory>
#include "nas/download_service/third_party/download_public_define.h"

namespace nas {
  const char* GetDownloadErrorDesc(DownloadErrorCode error_no);
} // namespace nas

#endif // NAS_DOWNLOAD_SERVER_SRC_MEDIA_ERROR_DESC_H_