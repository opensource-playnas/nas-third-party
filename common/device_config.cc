/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-26 11:27:44
 */

#include "nas/common/device_config.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/network_interfaces.h"

#include "nas/common/json_cfg/json_cfg.h"
#include "nas/common/nas_config.h"

#if BUILDFLAG(IS_WIN)
#include <Windows.h>
#endif

namespace nas {
namespace device {

const base::FilePath::CharType kNasConfigPath[] = FILE_PATH_LITERAL("nas.json");

const char* kServiceIdKey = "service_id";
const char* kWrapTokenKey = "wrap_token";
const char* kDeviceIdKey = "device_id";
const char* kDeviceOnwerKey = "device_owner";
const char* kDeviceStatus = "device_status";
const char* kNidKey = "nid";
const char* kNasNameKey = "nas_name";
const char* kNasVersionKey = "version";

const char* kNasInstanceId = "instance_id";
const char* kNasDistributionId = "distribution_id";
const char* kNasResDistributionId = "res_distribution_id";

base::FilePath GetNasJsonFilePath(unsigned short level) {
  // 不可能有太多层级,反而会导致死循环
  DCHECK(level < 255);

  base::FilePath module_path;
  base::PathService::Get(base::DIR_EXE, &module_path);
  // 0 是当前目录层级
  while (level--) {
    module_path = module_path.DirName();
  }

  base::FilePath nas_json_path = module_path.Append(kNasConfigPath);
  LOG(INFO) << "nas_json_path:" << nas_json_path;
  return nas_json_path;
}

std::string GetNasVersion(unsigned short level) {
  base::FilePath nas_json_path = GetNasJsonFilePath(level);
  if (!base::PathExists(nas_json_path)) {
    return "";
  }
  std::string nas_version;
  nas::NasConfigGetString(nas_json_path, kNasVersionKey, &nas_version, "",
                          false);
  return nas_version;
}

std::string GetNid(unsigned short level) {
  std::string nid;

  base::FilePath global_cfg_dir = nas::GetGlobalConfigDir();
  base::FilePath nid_path =
      global_cfg_dir.Append(base::FilePath::FromUTF8Unsafe("nid.json"));
  LOG(INFO) << "nid_path:" << nid_path;
  nas::NasConfigGetString(nid_path, kNidKey, &nid, "", false);
  return nid;
}

std::string GetInstanceId(unsigned short level) {
  base::FilePath nas_json_path = GetNasJsonFilePath(level);
  if (!base::PathExists(nas_json_path)) {
    return "";
  }
  std::string instance_id;
  nas::NasConfigGetString(nas_json_path, kNasInstanceId, &instance_id, "",
                          false);
  return instance_id;
}

std::string GetDistributionId(unsigned short level) {
  base::FilePath nas_json_path = GetNasJsonFilePath(level);
  if (!base::PathExists(nas_json_path)) {
    return "";
  }
  std::string distribution_id;
  nas::NasConfigGetString(nas_json_path, kNasDistributionId, &distribution_id,
                          "", false);
  return distribution_id;
}

std::string GetResDistributionId(unsigned short level) {
  base::FilePath nas_json_path = GetNasJsonFilePath(level);
  if (!base::PathExists(nas_json_path)) {
    return "";
  }
  std::string res_distribution_id;
  nas::NasConfigGetString(nas_json_path, kNasResDistributionId,
                          &res_distribution_id, "", false);
  return res_distribution_id;
}

std::string GetDeviceId() {
  base::FilePath device_json_path = GetDataDir();
  device_json_path =
      device_json_path.Append(FILE_PATH_LITERAL("global_config\\device.json"));
  if (!base::PathExists(device_json_path)) {
    return "";
  }
  std::string device_id;
  NasConfigGetString(device_json_path, "device_id", &device_id, "", false);
  return device_id;
}

}  // namespace device
}  // namespace nas