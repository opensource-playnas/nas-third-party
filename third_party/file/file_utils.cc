// copyright 2020 The Master Lu PC-Group Authors. All rights reserved.
// author guopengwei@ludashi.com
// date 2020/06/30 11:52
#include <LdsUtils/file_utils.h>

#include <atlstr.h>
#pragma warning(disable : 4091)
#include <shlobj.h>
#pragma warning(default : 4091)
#include <assert.h>
#include <strsafe.h>

#include <aclapi.h>
#include <sddl.h>

namespace file_utils {

BOOL ReadFileToStrBuf(const wchar_t* file_path, std::string* buf) {
  if (!file_path || !buf || !PathFileExists(file_path) ||
      PathIsDirectory(file_path)) {
    return FALSE;
  }

  BOOL ret = FALSE;
  HANDLE file_handle = INVALID_HANDLE_VALUE;

  do {
    file_handle =
        ::CreateFile(file_path, GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
      assert(FALSE);
      break;
    }

    LARGE_INTEGER file_size = {0};
    if (!::GetFileSizeEx(file_handle, &file_size)) {
      assert(FALSE);
      break;
    }

    if (file_size.HighPart != 0) {
      assert(!L"file too large!");
      break;
    }

    try {
      buf->resize(file_size.LowPart);
    } catch (...) {
      break;
    }

    DWORD num_ber_of_bytes_read = 0;
    ::ReadFile(file_handle, (BYTE*)&((*buf)[0]), file_size.LowPart,
               &num_ber_of_bytes_read, NULL);
    if (file_size.LowPart != num_ber_of_bytes_read) {
      break;
    }

    ret = TRUE;

  } while (FALSE);

  if (file_handle != INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle);
    file_handle = INVALID_HANDLE_VALUE;
  }

  return ret;
}

BOOL WriteFileFromBuf(const wchar_t* file_path, LPBYTE buff, DWORD buffer_len) {
  ATLASSERT(buff && buffer_len);
  if (nullptr == buff || buffer_len == 0)
    return FALSE;

  if (!file_path) {
    return FALSE;
  }

  BOOL ret = FALSE;
  HANDLE file_handle = INVALID_HANDLE_VALUE;

  do {
    file_handle = ::CreateFile(file_path, GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
      ATLASSERT(FALSE);
      break;
    }

    DWORD number_of_bytes_written = 0;
    ::WriteFile(file_handle, buff, buffer_len, &number_of_bytes_written, NULL);
    if (buffer_len != number_of_bytes_written) {
      break;
    }

    ret = TRUE;

  } while (FALSE);

  if (file_handle != INVALID_HANDLE_VALUE) {
    CloseHandle(file_handle);
    file_handle = INVALID_HANDLE_VALUE;
  }

  return ret;
}

void GetFilesA(LPCSTR dir,
               BOOL sub_dir,
               std::function<void(const WIN32_FIND_DATAA& find_data,
                                  BOOL* stop)> file_info_functor) {
  HANDLE find_handle = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATAA find_data{0};
  ATL::CStringA find_dir = dir;
  find_dir += "/*.*";
  find_handle = FindFirstFileA(find_dir, &find_data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    return;
  }

  do {
    if (strcmp(find_data.cFileName, ".") == 0 ||
        strcmp(find_data.cFileName, "..") == 0) {
      continue;
    }

    if (file_info_functor) {
      BOOL stop = FALSE;
      file_info_functor(find_data, &stop);
      if (stop) {
        break;
      }
    }

    if (sub_dir && (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      CHAR path[MAX_PATH] = {0};
      StringCchCopyA(path, MAX_PATH, dir);
      PathAppendA(path, find_data.cFileName);
      GetFilesA(path, sub_dir, file_info_functor);
    }

  } while (FindNextFileA(find_handle, &find_data) != 0);

  FindClose(find_handle);
  find_handle = INVALID_HANDLE_VALUE;
}

void GetFilesW(LPCWSTR dir,
               BOOL sub_dir,
               std::function<void(const WIN32_FIND_DATAW& find_data,
                                  BOOL* stop)> file_info_functor) {
  HANDLE find_handle = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATAW find_data{0};
  ATL::CStringW find_dir = dir;
  find_dir += L"/*.*";
  find_handle = FindFirstFileW(find_dir, &find_data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    return;
  }

  do {
    if (wcscmp(find_data.cFileName, L".") == 0 ||
        wcscmp(find_data.cFileName, L"..") == 0) {
      continue;
    }

    if (file_info_functor) {
      BOOL stop = FALSE;
      file_info_functor(find_data, &stop);
      if (stop) {
        break;
      }
    }

    if (sub_dir && (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      WCHAR path[MAX_PATH] = {0};
      StringCchCopyW(path, MAX_PATH, dir);
      PathAppendW(path, find_data.cFileName);
      GetFilesW(path, sub_dir, file_info_functor);
    }

  } while (FindNextFileW(find_handle, &find_data) != 0);

  FindClose(find_handle);
  find_handle = INVALID_HANDLE_VALUE;
}

void GetFiles(LPCTSTR dir,
              BOOL sub_dir,
              std::function<void(const WIN32_FIND_DATA& find_data, BOOL* stop)>
                  file_info_functor) {
#ifdef UNICODE
  GetFilesW(dir, sub_dir, file_info_functor);
#else
  GetFilesA(dir, sub_dir, file_info_functor);
#endif
}

int AddAccessToNetWorkService(LPCTSTR dir) {
  DWORD dwRes = 0;
  PACL pOldDACL = NULL, pNewDACL = NULL;
  PSECURITY_DESCRIPTOR pSD = NULL;
  EXPLICIT_ACCESS ea;
  if (GetNamedSecurityInfo(dir, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                           nullptr, nullptr, &pOldDACL, nullptr,
                           &pSD) != ERROR_SUCCESS) {
    // 处理错误
    return 1;
  }

  // 添加一个访问控制项
  ZeroMemory(&ea, sizeof(EXPLICIT_ACCESSW));
  ea.grfAccessPermissions = GENERIC_ALL;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
  ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
  ea.Trustee.ptstrName = const_cast<LPTSTR>(L"NETWORK SERVICE");

  PACL pNewAcl = nullptr;
  if (SetEntriesInAcl(1, &ea, pOldDACL, &pNewAcl) != ERROR_SUCCESS) {
    // 处理错误
    LocalFree(pSD);
    return 1;
  }

  // 更新目录的安全设置
  if (SetNamedSecurityInfo(const_cast<LPTSTR>(dir), SE_FILE_OBJECT,
                           DACL_SECURITY_INFORMATION, nullptr, nullptr, pNewAcl,
                           nullptr) != ERROR_SUCCESS) {
    // 处理错误
    LocalFree(pSD);
    LocalFree(pNewAcl);
    return 1;
  }

  // 释放内存
  LocalFree(pSD);
  LocalFree(pNewAcl);

  return 0;
}

std::wstring GetUserProfileDirectory(const std::wstring& sid) {
  HKEY hKey;
  std::wstring profileListPath =
      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\" + sid;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, profileListPath.c_str(), 0, KEY_READ,
                   &hKey) == ERROR_SUCCESS) {
    WCHAR profilePath[MAX_PATH];
    DWORD bufferSize = sizeof(profilePath);
    if (RegQueryValueEx(hKey, L"ProfileImagePath", nullptr, nullptr,
                        (LPBYTE)profilePath, &bufferSize) == ERROR_SUCCESS) {
    }
    RegCloseKey(hKey);
    return profilePath;
  }

  return L"";
}

};  // namespace file_utils