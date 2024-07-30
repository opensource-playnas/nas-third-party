/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-06-09 17:40:10
 */
#include "process_unit.h"

#include "base/files/file_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#if BUILDFLAG(IS_WIN)
#include <Psapi.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <strsafe.h>
#include <windows.h>
#elif BUILDFLAG(IS_LINUX)
#include <unistd.h>
#include <sys/types.h>
#endif

namespace nas {

namespace process_unit {

base::FilePath GetProcessFullPath(base::ProcessId pid) {
  base::FilePath::StringType full_path;
#if BUILDFLAG(IS_WIN)
  full_path = GetFullPath(pid);
  return base::FilePath(full_path);
#elif BUILDFLAG(IS_LINUX)
  full_path = GetFullPath(pid);
#endif
  return base::FilePath(full_path);
}

#if BUILDFLAG(IS_WIN)
std::wstring GetFullPath(DWORD pid) {
  wchar_t szImagePath[MAX_PATH] = {0};
  wchar_t pszFullPath[MAX_PATH] = {0};
  HANDLE hProcess = NULL;

  std::wstring ret;

  do {
    pszFullPath[0] = '\0';

    // 获取进程句柄失败
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, pid);
    if (!hProcess) {
      break;
    }

    // 获取进程完整路径失败
    if (!GetProcessImageFileName(hProcess, szImagePath, MAX_PATH)) {
      break;
    }

    // 路径转换失败
    if (!DosPathToNtPath(szImagePath, pszFullPath)) {
      break;
    }

    ret = pszFullPath;
  } while (false);

  if (hProcess) {
    CloseHandle(hProcess);
  }
  return ret;
}

BOOL DosPathToNtPath(LPWSTR dos_path, LPWSTR nt_path) {
  wchar_t szDriveStr[500] = {0};
  wchar_t szDrive[3] = {0};
  wchar_t szDevName[100] = {0};
  INT cchDevName;
  INT i;

  // 检查参数
  if (!dos_path || !nt_path)
    return FALSE;

  // 获取本地磁盘字符串
  if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr)) {
    for (i = 0; szDriveStr[i]; i += 4) {
      if (!lstrcmpi(&(szDriveStr[i]), L"A:\\") ||
          !lstrcmpi(&(szDriveStr[i]), L"B:\\")) {
        continue;
      }

      szDrive[0] = szDriveStr[i];
      szDrive[1] = szDriveStr[i + 1];
      szDrive[2] = '\0';
      // 查询 Dos 设备名
      if (!QueryDosDevice(szDrive, szDevName, 100)) {
        return FALSE;
      }

      // 命中
      cchDevName = lstrlen(szDevName);
      // 在某些电脑上出现\Device\HarddiskVolume11被
      // 替换为 \Device\HarddiskVolume1,导致把F盘转换为D盘
      // 需要判断为绝对相等

      std::wstring dos_path_str(dos_path);
      // 2 是取 \Device\HarddiskVolume1\ 第三个'\'
      std::wstring dev_name_str =
          GetSubstringBeforeNthDelimiter(dos_path_str, L"\\", 2);
      if (!dev_name_str.empty()) {
        if (dev_name_str.compare(szDevName) == 0) {
          // 复制驱动器
          lstrcpy(nt_path, szDrive);

          // 复制路径
          lstrcat(nt_path, dos_path + cchDevName);
          return TRUE;
        }
      }
    }
  }

  lstrcpy(nt_path, dos_path);

  return FALSE;
}

std::wstring GetSubstringBeforeNthDelimiter(const std::wstring& str,
                                            std::wstring delimiter,
                                            int n) {
  std::size_t pos = 0;
  int count = 0;

  while (count < n && pos != std::wstring::npos) {
    pos = str.find(delimiter, pos + 1);
    count++;
  }

  if (count == n && pos != std::wstring::npos) {
    return str.substr(0, pos);
  }

  return L"";
}

#elif BUILDFLAG(IS_LINUX)
std::string GetFullPath(pid_t pid) {
  char proc_path[256];
  snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", pid);

  char exe_path[4096];
  ssize_t length = readlink(proc_path, exe_path, sizeof(exe_path) - 1);

  if (length != -1) {
    exe_path[length] = '\0';
    return std::string(exe_path);
  } else {
    return "";
  }
}
#endif

};  // namespace path_unit

};  // namespace nas
