// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author heyiqian@ludashi.com
// date 2023-11-30 15:19:07

#ifndef NAS_COMMON_PATH_WIN_UTILS_H_
#define NAS_COMMON_PATH_WIN_UTILS_H_

#include <windows.h>

#include "profile_def.h"

namespace nas {
namespace path_unit {
void GetProfileList(HKEY hkey, ProfileInfoArr& profile_list);
void GetProfileListWin(AccountProfileInfoArr& account_profile_list);
}  // namespace path_unit
}  // namespace nas

#endif