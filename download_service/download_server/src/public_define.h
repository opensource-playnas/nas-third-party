/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: wanyan@ludashi.com
* @Date: 2023-09-04 19:09:13
*/
#ifndef NAS_DOWNLOAD_SERVER_SRC_PUBLIC_DEFINE_H_
#define NAS_DOWNLOAD_SERVER_SRC_PUBLIC_DEFINE_H_

namespace nas {
static const char* kNameSpace = "download";
// ----------------------------用户级------------------------------
static const char* kTaskLimitCountKey = "task_limit_count";
static const char* kPrioritySmallTaskSwitchKey = "priority_small_task_switch";
static const char* kSmallTaskSizeKey = "small_task_size";
static const char* kDownloadSpeedLimitSwitchKey = "download_speed_limit_switch";

// ----------------------------系统级------------------------------
static const char* kCurlMaxDownloadSpeedKey = "curl_max_download_speed";
static const char* kBtTaskCreateSubdirSwitchKey = "bt_task_create_subdir_switch";
static const char* kBtTaskAutoDownloadSwitchKey = "bt_task_auto_download_switch";
static const char* kBtTCPTListenPortKey = "bt_tcp_listen_port";
static const char* kBtMaxUploadSpeedKey = "bt_max_upload_speed";
static const char* kBtMaxDownloadSpeedKey = "bt_max_download_speed";
static const char* kBtEncryptionModeKey = "bt_encryption_mode";
static const char* kBtMaxConnectionsKey = "bt_max_connections";
static const char* kBtTaskMaxConnectionsKey = "bt_task_max_connections";
static const char* kBtEnableDHTSwitchKey = "bt_enable_dht_switch";
static const char* kBtEnableUPnPOrNATPortSwitchKey = "bt_enable_upnp_or_nat_port_switch";
//static const char* kBtShareRatioLimitSwitchKey = "bt_share_ratio_limit_switch";
static const char* kBtShareRatioLimitValueKey = "bt_share_ratio_limit_value";
//static const char* kBtSeedingTimeLimitSwitchKey = "bt_seeding_time_limit_switch";
static const char* kBtSeedingTimeLimitValueKey = "bt_seeding_time_limit_value";
static const char* kMuleTcpListenPortKey = "mule_tcp_listen_port";
static const char* kMuleUdpListenPortKey = "mule_udp_listen_port";
static const char* kMuleMaxConnectionsKey = "mule_max_connections";
static const char* kMuleMaxUploadSpeedKey = "mule_max_upload_speed";
static const char* kMuleMaxDownloadSpeedKey = "mule_max_download_speed";


}  // namespace nas
#endif // NAS_DOWNLOAD_SERVER_SRC_PUBLIC_DEFINE_H_