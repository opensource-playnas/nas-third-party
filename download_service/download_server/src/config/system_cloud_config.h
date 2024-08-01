/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-09-04 17:43:42
 */

#ifndef NAS_DOWNLOAD_SERVER_SRC_CONFIG_SYSTEM_CLOUD_CONFIG_H_
#define NAS_DOWNLOAD_SERVER_SRC_CONFIG_SYSTEM_CLOUD_CONFIG_H_

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/threading/thread.h"

#include "nas/download_service/download_server/src/download_helper/download_interface_helper.h"

namespace nas {

// enum SeedingTimeLimitMode {
//   kIgnore = 0,  // 忽略，即一直做种
//   kAlways,
//   kCustom
// };

class SystemCloudConfig {
 public:
  static SystemCloudConfig* GetInstance();
  SystemCloudConfig(const SystemCloudConfig&) = delete;
  SystemCloudConfig& operator=(const SystemCloudConfig&) = delete;

  void Init();
  int64_t GetCurlMaxDownloadSpeed() const;
  bool IsShareRatioLimit() const;
  int GetShareRatioLimitValue() const;
  bool IsSeedingTimeLimit() const;
  int GetSeedingTimeLimitValue() const;
  bool IsBtAutoDownload() const;

 private:
  void ParseSystemSettings();
  void OnSubscribeCallbackResult(const std::string& key,
                                 const std::string& value);
  void OnSubscribeComplete(bool success);
  void SubcribeRedis();
  void SetValue();

 private:
  // This object is a singleton:
  SystemCloudConfig();
  ~SystemCloudConfig();
  friend struct base::DefaultSingletonTraits<SystemCloudConfig>;

 private:
  // HTTP 和 FTP 设置， 单位：kb/每秒, 0代表不限制
  std::atomic<int64_t> curl_max_download_speed_ = 0;

  // bt设置：为每个任务创建专门用于保存其下载内容的子文件夹，开关类型，默认：开
  bool bt_task_create_subdir_switch_ = true;
  // bt设置：添加任务后是否自动开始下载， 开关类型，默认：开
  std::atomic<bool> bt_task_auto_download_switch_ = true;
  // bt设置: TCP 监听端口，默认值35226， 范围0~65535
  int bt_tcp_listen_port_ = 35226;
  // bt设置: 最大上传速度，单位：kb/每秒，范围0~1048576kb/s, 0代表不限制
  int64_t bt_max_upload_speed_ = 0;
  // bt设置: 最大下载速度，单位：kb/每秒，范围0~1048576kb/s, 0代表不限制
  int64_t bt_max_download_speed_ = 0;
  // bt设置: 加密模式，0：允许加密；1：强制加密；2：禁用加密
  EncryptionMode bt_encryption_mode_ = EncryptionMode::kAllow;
  // bt设置: 全局最大连接数，2~2000
  int bt_max_connections_ = 500;
  // bt设置: 每个torrent最大连接数，2~2000
  int bt_torrent_max_connections_ = 500;
  // bt设置: 启动 DHT 网络，开关类型，默认：开
  bool bt_enable_dht_switch_ = true;
  // bt设置: 启用 UPnP/NAT-PMP端口转发（需要路由器支持），开关类型，默认：开
  bool bt_enable_upnp_or_nat_port_switch_ = true;
  // bt设置: 做种限制
  // 分享率限制值， 0代表不限制
  std::atomic<int> bt_share_ratio_limit_value_ = 0;
  // 持续做种达到限制值，单位：分钟， 0代表不限制
  std::atomic<int> bt_seeding_time_limit_value_ = 0;

  // amule设置：监听的tpc端口， 范围为0~65532
  int mule_tcp_listen_port_ = 4662;
  // amule设置：监听的udp端口
  int mule_udp_listen_port_ = 4672;
  // amule设置：最大联机数量, 范围为5~7500
  int mule_max_connections_ = 480;
  // amule设置：最大上传速度，单位kb/s，范围0~1048576, 0代表不限制
  int mule_max_upload_speed_ = 0;
  // amule设置：最大下载速度，单位kb/s，范围0~1048576, 0代表不限制
  int mule_max_download_speed_ = 0;

  BtSettingInterface* bt_settings_ = nullptr;
  AmuleSettingInterface* amule_settings_ = nullptr;
};  // NAS_DOWNLOAD_SERVER_SRC_CONFIG_SYSTEM_CLOUD_CONFIG_H_
}  // namespace nas

#endif