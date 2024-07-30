// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/01/18 14:48

#ifndef NAS_DLNA_SERVER_SRC_UTILS_H_
#define NAS_DLNA_SERVER_SRC_UTILS_H_

#include <map>

#include "base/containers/linked_list.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "call_result.h"

#include "public_define.h"

namespace utils {
double Round(double number, unsigned int bits);

std::string GetFileExtensionFromUrl(const std::string& url_string);

}  // namespace utils

#endif  // NAS_DLNA_SERVER_SRC_UTILS_H_