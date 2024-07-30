// copyright 2022 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2022/09/14 16:01
#ifndef NAS_COMMON_COMMON_CONFIG_H_
#define NAS_COMMON_COMMON_CONFIG_H_

#include "base/files/file_path.h"
#include "base/logging.h"

namespace nas {
struct NasConf {
  std::string version;
  std::string entry_dir;
  std::string user_data_dir;
  std::string sid;
};

void InitLog(const std::string& log_file,
             int log_min_level = logging::LOG_VERBOSE);

// 获取 Nas 安装的根目录
base::FilePath GetNasAppInstallDir();

// 获取Nas 的web 资源配置目录
// relative_level: 当前exe相对安装 openresty 的层级
base::FilePath GetNasWebResourceDir(int relative_level = 1);

// TODO 暂时弃用
// web 无需鉴权可直接访问的目录
// relative_level: 当前exe相对安装 openresty 的层级
base::FilePath GetWebDirectAccessDir(int relative_level = 1);

// 获取 Nas 缓存数据目录
// 日志/配置/数据 等都存放在该目录下
base::FilePath GetNasHomeDir();

// 获取Nas 配置 目录
base::FilePath GetNasConfigDir();

// 获取Nas 日志 目录
base::FilePath GetNasLogDir();

// 获取Nas dump 目录
base::FilePath GetNasDumpDir();

// 读取nas.conf内容
void ReadNasConf(unsigned short level, NasConf& nas_conf);

std::string GetNasEntryDir();

// 获取数据目录
base::FilePath GetDataDir();

// 获取用户的源数据目录,如果不存在对应的用户目录则create_if_not_exists判断是否创建
base::FilePath GetUserSourceDataDir(const std::string& user_id, bool create_if_not_exists);

// 和用户源数据目录和非单个用户无关的目录
base::FilePath GetPublicSourceDataDir();

// 全局配置目录
base::FilePath GetGlobalConfigDir();

// 全局缓存目录
base::FilePath GetNasCacheDir();

// 套件服务配置存放目录
base::FilePath GetAppConfigDir();

// 指定目录下创建子目录,失败返回空目录
base::FilePath CreateSubDir(base::FilePath parent,
                            base::FilePath::StringPieceType sub_name);
base::FilePath GetPublicUserDataDir(const base::FilePath& kInstallDir);

base::FilePath GetPublicUserDataCacheDir(const base::FilePath& kInstallDir);

base::FilePath GetNasJsonFilePath(unsigned short level);

// 获取7z.dll路径
base::FilePath Get7zFilePath(unsigned short level);
base::FilePath Get7zaFilePath(unsigned short level);

std::string ConvertNasEnvironment(const std::string& input,
                                  const std::string& user_id);

bool IsAutoTestModel();

#if BUILDFLAG(IS_WIN)
std::string GetProfilePath(const std::string& sid);
#elif BUILDFLAG(IS_LINUX)
#endif

}  // namespace nas

#endif  // NAS_COMMON_COMMON_CONFIG_H_