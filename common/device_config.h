/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-26 11:27:16
 */

#ifndef NAS_COMMON_DEICE_CONFIG_H_
#define NAS_COMMON_DEICE_CONFIG_H_

#include <string>
#include <base/files/file_path.h>

namespace nas {
namespace device {
base::FilePath GetNasJsonFilePath(unsigned short level);
    // 获取 nas 的版本号
std::string GetNasVersion(unsigned short level);

// 获取nid,level值为模块相对于根目录层级
std::string GetNid(unsigned short level);


std::string GetInstanceId(unsigned short level);
std::string GetDistributionId(unsigned short level);
std::string GetResDistributionId(unsigned short level);



// 获取设备id
std::string GetDeviceId();

}  // namespace device

}  // namespace nas

#endif  // NAS_COMMON_DEICE_CONFIG_H_
