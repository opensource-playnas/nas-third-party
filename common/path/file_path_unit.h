
#ifndef NAS_COMMON_PATH_FILE_PATH_UNIT_H_
#define NAS_COMMON_PATH_FILE_PATH_UNIT_H_

#include <string>
#include <map>

#include "base/files/file_path.h"
#include "profile_def.h"

namespace nas {

namespace path_unit {

enum FileType : int64_t {
  KUnknown = 0,
  kAll = 1,             // 所有类型
  kUndefine = 1 << 1,   // 未定义类型
  kDriver = 1 << 2,     // 驱动器
  kDirectory = 1 << 3, // 目录
  kTxt = 1 << 4,  // 文档 txt
  kPdf = 1 << 5,   // 文档 pdf
  kWord = 1 << 6,      // 文档 word
  kPPT= 1 << 7,      // 文档 ppt
  kXlxs= 1 << 8,      // 文档 xlxs
  kHtml = 1 << 9, // html
  kCompress = 1 << 10,   // 压缩文件
  kPicture = 1 << 11,    // 图片
  kVideo = 1 << 12,    // 视频
  kMusic = 1 << 13, // 音乐
  kApk = 1 << 14, // apk
  kExe = 1 << 15, // exe
  kBt = 1 << 16, // bt
};

// 将 '\\' 转换为 '/'
base::FilePath UnifiedSlash(const base::FilePath& src_path);
std::string UnifiedSlash(const std::string& src_path);

// 替换掉路径中的双斜杠 // 为 /
base::FilePath RemoveDoubleSlash(const base::FilePath& src_path);
std::string RemoveDoubleSlash(const std::string& src_path);

// 将 '/' 转换为 '\\'
std::string ToWindowsSlash(const std::string& src_path);
base::FilePath ToWindowsSlash(const base::FilePath& src_path);

// 在指定文件夹下，根据指定文件名(exist_path)生成一个不存在的文件名 至多尝试10000次
// 若指定文件名(exist_path)不存在， 直接使用指定文件名
base::FilePath GetNoExistFileNameInSpecDir(const base::FilePath& exist_path);
// 某些场景 增加“副本”是不合适的, 追加 (2) ... (3) 才合适
// 对GetNoExistFileNameInSpecDir的扩展，only_number未false则是GetNoExistFileNameInSpecDir的功能
base::FilePath GetNoExistFilePath(const base::FilePath& exist_path,
                                  bool only_number);
base::FilePath AddExtension(const base::FilePath& src_path, const std::string& ext);

// 移除路径后面的 '\\' 或者 '/'
// 考虑到linux下 root 既是 /, 处理时会判断 path length 是否 > 1
void RemoveLastSlash(std::string& path);

// win ， 通过 base::FilePath::FromUTF8Unsafe 构造
// posix ， 直接构造
// convert_slash 为 true 会将斜杠转换为 /
base::FilePath BaseFilePathFromU8(const std::string& path,
                                  bool convert_slash = true);
// 功能同 BaseFilePathFromU8
// 区别: win 下如果路径超过 MAX_PATH 会忽略 convert_slash 参数, 同时会增加
// \\?\ 前缀.
// 备注: Windows 10, Version 1607 之前的版本不支持通过设置支持超过MAX_PATH. 如果
// \ 转换成了 /, \\?\ 前缀无效.
base::FilePath BaseFilePathFromU8EX(const std::string& path,
                                    bool convert_slash = true);
std::string BaseFilePathToU8(const base::FilePath& path);

// 校正路径
void CorrectionPath(base::FilePath& path);

FileType GetFileType(const std::string& file_name,
                     bool convert_to_lower = true);

#if BUILDFLAG(IS_WIN)
bool HasNetworkServiceReadWriteAccess(const std::wstring& filePath);
// 给目录新增 network_service 用户访问权限
int AddAccessToNetWorkService(const base::FilePath& dir);
// 根据磁盘盘符获取硬件BUG总线
int32_t GetBusType(char letter);
bool IsUSB(char letter);
#endif

// full_equal: true, path1 是否和 path2 相等
//             false, path1 包含 path2且以 path2 开始
bool MatchPath(const std::string& path1,
               const std::string& path2,
               bool full_equal);

void GetProfileList(AccountProfileInfoArr& profile_list);

};  // namespace path_unit
};  // namespace nas
#endif 