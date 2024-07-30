/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-06-09 17:40:02
 */
#ifndef NAS_COMMON_PROCESS_PROCESS_UNIT_H_
#define NAS_COMMON_PROCESS_PROCESS_UNIT_H_

#include "base/files/file_path.h"
#include "base/process/process.h"
#include "base/process/process_iterator.h"

namespace nas {

namespace process_unit {
base::FilePath GetProcessFullPath(base::ProcessId pid);

#if BUILDFLAG(IS_WIN)
std::wstring GetFullPath(DWORD pid);
BOOL DosPathToNtPath(LPWSTR dos_path, LPWSTR nt_path);
std::wstring GetSubstringBeforeNthDelimiter(const std::wstring& str,
                                            std::wstring delimiter,
                                            int n);

#elif BUILDFLAG(IS_LINUX)
std::string GetFullPath(pid_t pid);
#endif

};      // namespace process_unit
};      // namespace nas
#endif  // NAS_COMMON_PROCESS_PROCESS_UNIT_H_