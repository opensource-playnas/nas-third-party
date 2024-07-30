#ifndef LDS_UTILS_FILE_UTILS_H_
#define LDS_UTILS_FILE_UTILS_H_

#include <Windows.h>
#include <string>
#include <functional>

namespace file_utils {
BOOL ReadFileToStrBuf(const wchar_t * file_path, std::string *buf);

BOOL WriteFileFromBuf(const wchar_t *file_path, LPBYTE buf,
                      DWORD buffer_len);

void GetFilesA(
    LPCSTR dir, BOOL sub_dir,
    std::function<void(const WIN32_FIND_DATAA& find_data, BOOL* stop)>
        file_info_functor);

void GetFilesW(
    LPCWSTR dir, BOOL sub_dir,
    std::function<void(const WIN32_FIND_DATAW& find_data, BOOL* stop)>
        file_info_functor);

void GetFiles(LPCTSTR dir, BOOL sub_dir,
              std::function<void(const WIN32_FIND_DATA& find_data, BOOL* stop)>
                  file_info_functor);

int AddAccessToNetWorkService(LPCTSTR dir);

std::wstring GetUserProfileDirectory(const std::wstring& sid);


};  // namespace FileUtils

#endif  // LDS_UTILS_FILE_UTILS_H_
