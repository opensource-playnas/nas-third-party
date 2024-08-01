/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-09-04 17:43:28
 */

#include "system_cloud_config.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/task/thread_pool.h"

#include "nas/common/nas_config.h"
#include "nas/common/path/file_path_unit.h"
#include "nas/download_service/download_server/src/public_define.h"
#include "nas/download_service/download_server/src/utils/utils.h"

namespace nas {

SystemCloudConfig* SystemCloudConfig::GetInstance() {
  return base::Singleton<SystemCloudConfig>::get();
}

void SystemCloudConfig::Init() {
  bt_settings_ =
      DownloadInterfaceHelper::GetInstance()->GetBtSettingsInterface();
  amule_settings_ =
      DownloadInterfaceHelper::GetInstance()->GetAmuleSettingsInterface();
  ParseSystemSettings();
  SubcribeRedis();
}

int64_t SystemCloudConfig::GetCurlMaxDownloadSpeed() const {
  return curl_max_download_speed_.load();
}

bool SystemCloudConfig::IsShareRatioLimit() const {
  return (bt_share_ratio_limit_value_.load() != 0) ? true : false;
}

int SystemCloudConfig::SystemCloudConfig::GetShareRatioLimitValue() const {
  return bt_share_ratio_limit_value_.load();
}

bool SystemCloudConfig::IsSeedingTimeLimit() const {
  return (bt_seeding_time_limit_value_.load() != 0) ? true : false;
}

int SystemCloudConfig::GetSeedingTimeLimitValue() const {
  return bt_seeding_time_limit_value_.load();
}

bool SystemCloudConfig::IsBtAutoDownload() const {
  return bt_task_auto_download_switch_.load();
}

void SystemCloudConfig::ParseSystemSettings() {
  base::Value::Dict root_dict;
  base::Value* data = root_dict.Find(kNameSpace);
  if (data) {
    base::Value::Dict* data_dict = data->GetIfDict();
    if (data_dict) {
      std::string* curl_max_download_speed =
          data_dict->FindString(kCurlMaxDownloadSpeedKey);
      std::string* bt_task_create_subdir_switch =
          data_dict->FindString(kBtTaskCreateSubdirSwitchKey);
      std::string* bt_task_auto_download_switch =
          data_dict->FindString(kBtTaskAutoDownloadSwitchKey);
      std::string* bt_tcp_listen_port =
          data_dict->FindString(kBtTCPTListenPortKey);
      std::string* bt_max_upload_speed =
          data_dict->FindString(kBtMaxUploadSpeedKey);
      std::string* bt_max_download_speed =
          data_dict->FindString(kBtMaxDownloadSpeedKey);
      std::string* bt_encryption_mode =
          data_dict->FindString(kBtEncryptionModeKey);
      std::string* bt_max_connections =
          data_dict->FindString(kBtMaxConnectionsKey);
      std::string* bt_task_max_connections =
          data_dict->FindString(kBtTaskMaxConnectionsKey);
      std::string* bt_enable_dht_switch =
          data_dict->FindString(kBtEnableDHTSwitchKey);
      std::string* bt_enable_upnp_or_nat_port_switch =
          data_dict->FindString(kBtEnableUPnPOrNATPortSwitchKey);
      std::string* bt_share_ratio_limit_value =
          data_dict->FindString(kBtShareRatioLimitValueKey);
      std::string* bt_seeding_time_limit_value =
          data_dict->FindString(kBtSeedingTimeLimitValueKey);

      std::string* mule_tcp_listen_port =
          data_dict->FindString(kMuleTcpListenPortKey);
      std::string* mule_udp_listen_port =
          data_dict->FindString(kMuleUdpListenPortKey);
      std::string* mule_max_connections =
          data_dict->FindString(kMuleMaxConnectionsKey);
      std::string* mule_max_upload_speed =
          data_dict->FindString(kMuleMaxUploadSpeedKey);
      std::string* mule_max_download_speed =
          data_dict->FindString(kMuleMaxDownloadSpeedKey);

      if (curl_max_download_speed) {
        curl_max_download_speed_.store(
            nas::utils::StringToInt(*curl_max_download_speed));
      }
      if (bt_task_create_subdir_switch) {
        bt_task_create_subdir_switch_ =
            nas::utils::StringToInt(*bt_task_create_subdir_switch);
      }

      if (bt_task_auto_download_switch) {
        bt_task_auto_download_switch_.store(
            nas::utils::StringToInt(*bt_task_auto_download_switch));
      }
      if (bt_tcp_listen_port) {
        bt_tcp_listen_port_ = nas::utils::StringToInt(*bt_tcp_listen_port);
      }
      if (bt_max_upload_speed) {
        bt_max_upload_speed_ = nas::utils::StringToInt(*bt_max_upload_speed);
      }

      if (bt_max_download_speed) {
        bt_max_download_speed_ =
            nas::utils::StringToInt(*bt_max_download_speed);
      }
      if (bt_encryption_mode) {
        bt_encryption_mode_ = static_cast<EncryptionMode>(
            nas::utils::StringToInt(*bt_encryption_mode));
      }
      if (bt_max_connections) {
        bt_max_connections_ = nas::utils::StringToInt(*bt_max_connections);
      }
      if (bt_task_max_connections) {
        bt_torrent_max_connections_ =
            nas::utils::StringToInt(*bt_task_max_connections);
      }
      if (bt_enable_dht_switch) {
        bt_enable_dht_switch_ = nas::utils::StringToInt(*bt_enable_dht_switch);
      }
      if (bt_enable_upnp_or_nat_port_switch) {
        bt_enable_upnp_or_nat_port_switch_ =
            nas::utils::StringToInt(*bt_enable_upnp_or_nat_port_switch);
      }
      if (bt_share_ratio_limit_value) {
        bt_share_ratio_limit_value_.store(
            nas::utils::StringToInt(*bt_share_ratio_limit_value));
      }
      if (bt_seeding_time_limit_value) {
        bt_seeding_time_limit_value_.store(
            nas::utils::StringToInt(*bt_seeding_time_limit_value));
      }

      if (mule_tcp_listen_port) {
        mule_tcp_listen_port_ = nas::utils::StringToInt(*mule_tcp_listen_port);
      }
      if (mule_udp_listen_port) {
        mule_udp_listen_port_ = nas::utils::StringToInt(*mule_udp_listen_port);
      }
      if (mule_max_connections) {
        mule_max_connections_ = nas::utils::StringToInt(*mule_max_connections);
      }
      if (mule_max_upload_speed) {
        mule_max_upload_speed_ =
            nas::utils::StringToInt(*mule_max_upload_speed);
      }
      if (mule_max_download_speed) {
        mule_max_download_speed_ =
            nas::utils::StringToInt(*mule_max_download_speed);
      }
    }
    SetValue();
  }
}

void SystemCloudConfig::OnSubscribeCallbackResult(const std::string& key,
                                                  const std::string& value) {
  LOG(INFO) << "subscribe callback result: key =" << key
            << ", value = " << value;
  if (key.find(kCurlMaxDownloadSpeedKey) != std::string::npos) {
    curl_max_download_speed_.store(nas::utils::StringToInt(value));
  } else if (key.find(kBtTaskCreateSubdirSwitchKey) != std::string::npos) {
    bt_task_create_subdir_switch_ = value == "1";
    bt_settings_->EnableCreateSubdir(bt_task_create_subdir_switch_);
  } else if (key.find(kBtTaskAutoDownloadSwitchKey) != std::string::npos) {
    bt_task_auto_download_switch_.store(value == "1");
    bt_settings_->EnableAutoDownload(bt_task_auto_download_switch_);
  } else if (key.find(kBtTCPTListenPortKey) != std::string::npos) {
    bt_tcp_listen_port_ = nas::utils::StringToInt(value);
    bt_settings_->SetTcpListenPort(bt_tcp_listen_port_);
  } else if (key.find(kBtMaxDownloadSpeedKey) != std::string::npos) {
    bt_max_download_speed_ = nas::utils::StringToInt(value);
    bt_settings_->SetMaxDownloadSpeed(bt_max_download_speed_);
  } else if (key.find(kBtMaxUploadSpeedKey) != std::string::npos) {
    bt_max_upload_speed_ = nas::utils::StringToInt(value);
    bt_settings_->SetMaxUploadSpeed(bt_max_upload_speed_);
  } else if (key.find(kBtEncryptionModeKey) != std::string::npos) {
    bt_encryption_mode_ =
        static_cast<EncryptionMode>(nas::utils::StringToInt(value));
    bt_settings_->SetEncryptionMode(bt_encryption_mode_);
  } else if (key.find(kBtMaxConnectionsKey) != std::string::npos) {
    bt_max_connections_ = nas::utils::StringToInt(value);
    bt_settings_->SetMaxConnections(bt_max_connections_);
  } else if (key.find(kBtTaskMaxConnectionsKey) != std::string::npos) {
    bt_torrent_max_connections_ = nas::utils::StringToInt(value);
    bt_settings_->SetTorrentMaxConnections(bt_torrent_max_connections_);
  } else if (key.find(kBtEnableDHTSwitchKey) != std::string::npos) {
    bt_enable_dht_switch_ = value == "1";
    bt_settings_->EnableDHT(bt_enable_dht_switch_);
  } else if (key.find(kBtEnableUPnPOrNATPortSwitchKey) != std::string::npos) {
    bt_enable_upnp_or_nat_port_switch_ = value == "1";
    bt_settings_->EnableUPnPAndNat(bt_enable_upnp_or_nat_port_switch_);
  } else if (key.find(kBtShareRatioLimitValueKey) != std::string::npos) {
    bt_share_ratio_limit_value_.store(nas::utils::StringToInt(value));
  } else if (key.find(kBtSeedingTimeLimitValueKey) != std::string::npos) {
    bt_seeding_time_limit_value_.store(nas::utils::StringToInt(value));
  } else if (key.find(kMuleTcpListenPortKey) != std::string::npos) {
    mule_tcp_listen_port_ = nas::utils::StringToInt(value);
    amule_settings_->SetTcpListenPort(mule_tcp_listen_port_);
  } else if (key.find(kMuleUdpListenPortKey) != std::string::npos) {
    mule_udp_listen_port_ = nas::utils::StringToInt(value);
    amule_settings_->SetUdpListenPort(mule_udp_listen_port_);
  } else if (key.find(kMuleMaxConnectionsKey) != std::string::npos) {
    mule_max_connections_ = nas::utils::StringToInt(value);
    amule_settings_->SetMaxConnections(mule_max_connections_);
  } else if (key.find(kMuleMaxUploadSpeedKey) != std::string::npos) {
    mule_max_upload_speed_ = nas::utils::StringToInt(value);
    amule_settings_->SetMaxUploadSpeed(mule_max_upload_speed_);
  } else if (key.find(kMuleMaxDownloadSpeedKey) != std::string::npos) {
    mule_max_download_speed_ = nas::utils::StringToInt(value);
    amule_settings_->SetMaxDownloadSpeed(mule_max_download_speed_);
  }
}

void SystemCloudConfig::SetValue() {
  if (bt_settings_) {
    bt_settings_->EnableCreateSubdir(bt_task_create_subdir_switch_);
    bt_settings_->EnableAutoDownload(bt_task_auto_download_switch_.load());
    bt_settings_->SetTcpListenPort(bt_tcp_listen_port_);
    bt_settings_->SetMaxDownloadSpeed(bt_max_download_speed_);
    bt_settings_->SetMaxUploadSpeed(bt_max_upload_speed_);
    bt_settings_->SetEncryptionMode(bt_encryption_mode_);
    bt_settings_->SetMaxConnections(bt_max_connections_);
    bt_settings_->SetTorrentMaxConnections(bt_torrent_max_connections_);
    bt_settings_->EnableDHT(bt_enable_dht_switch_);
    bt_settings_->EnableUPnPAndNat(bt_enable_upnp_or_nat_port_switch_);
    bt_settings_->SetPreallocationEnabled(false);
  }

  if (amule_settings_) {
    amule_settings_->SetTcpListenPort(mule_tcp_listen_port_);
    amule_settings_->SetUdpListenPort(mule_udp_listen_port_);
    amule_settings_->SetMaxConnections(mule_max_connections_);
    amule_settings_->SetMaxUploadSpeed(mule_max_upload_speed_);
    amule_settings_->SetMaxDownloadSpeed(mule_max_download_speed_);
  }
}

void SystemCloudConfig::OnSubscribeComplete(bool success) {
  LOG(INFO) << __func__ << ", result: " << success;
  if (!success) {
    base::ThreadPool::PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&SystemCloudConfig::SubcribeRedis,
        base::Unretained(this)),
      base::Milliseconds(2000));
  }
}

void SystemCloudConfig::SubcribeRedis() {
}

SystemCloudConfig::SystemCloudConfig() {}

SystemCloudConfig::~SystemCloudConfig() {}

}  // namespace nas
