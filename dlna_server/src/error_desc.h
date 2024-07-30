/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-15 11:15:55
 */

#ifndef NAS_DLNA_SERVER_SRC_MEDIA_ERROR_DESC_H_
#define NAS_DLNA_SERVER_SRC_MEDIA_ERROR_DESC_H_

#include "public_define.h"

namespace nas {
const char* GetErrorDesc(DlnaServiceErrorNo error_no);

}  // namespace nas

#endif  // NAS_DLNA_SERVER_SRC_MEDIA_ERROR_DESC_H_