/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-18 11:09:54
 */

#include "user_info.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/task/thread_pool.h"

#include "nas/download_service/download_server/src/utils/utils.h"
#include "nas/download_service/download_server/src/public_define.h"

namespace nas {
UserInfo::UserInfo(const std::string& user_id):user_id_(user_id){
  ParseAndSubscribe(user_id);
}

UserInfo::UserInfo(const UserInfo& other) {
  statistics_info_ = other.statistics_info_;
  context_field_ = other.context_field_;
  task_limit_count_ = other.task_limit_count_.load();
  priority_small_task_switch_ = other.priority_small_task_switch_.load();
  small_task_size_ = other.small_task_size_.load();
}

void UserInfo::ParseAndSubscribe(const std::string& user_id) {
  ParseUserSettings(user_id);
  SubcribeRedis();
}

void UserInfo::SetDownloadStatisticsInfo(
    const DownloadStatisticsInfo& statistics_info) {
  statistics_info_ = statistics_info;
}
DownloadStatisticsInfo UserInfo::GetDownloadStatisticsInfo() {
  return statistics_info_;
}

void UserInfo::SetContextField(const ContextField& context_field) {
  context_field_ = context_field;
}
ContextField UserInfo::GetContextField() {
  return context_field_;
}

void UserInfo::ParseUserSettings(const std::string& user_id) {
  base::Value::Dict root_dict ;
  base::Value* data = root_dict.Find(kNameSpace);
  if (data) {
    base::Value::Dict* data_dict = data->GetIfDict();
    if (data_dict) {
      std::string* task_limit_count = data_dict->FindString(kTaskLimitCountKey);
      std::string* priority_small_task_switch =
          data_dict->FindString(kPrioritySmallTaskSwitchKey);
      std::string* small_task_size = data_dict->FindString(kSmallTaskSizeKey);

      if (task_limit_count) {
        int count = nas::utils::StringToInt(*task_limit_count);
        if (count > 0) {
          task_limit_count_.store(count);
        }
      }

      if (priority_small_task_switch) {
        priority_small_task_switch_.store(
            nas::utils::StringToInt(*priority_small_task_switch));
      }

      if (small_task_size) {
        small_task_size_.store(nas::utils::StringToInt(*small_task_size));
      }
    }
  }
}

uint32_t UserInfo::GetTaskLimitCount() {
  return task_limit_count_.load();
}

bool UserInfo::IsPrioritySmallLimitTask() {
  return priority_small_task_switch_.load();
}

int64_t UserInfo::GetSmallTaskSize() {
  return small_task_size_.load();
}

void UserInfo::OnSubscribeCallbackResult(const std::string& key,
                                         const std::string& value) {
  LOG(INFO) << "subscribe callback result: key =" << key
            << ", value = " << value;
  if (key.find(kTaskLimitCountKey) != std::string::npos) {
    int limit_count = nas::utils::StringToInt(value);
    if (limit_count > 0) {
        task_limit_count_.store(limit_count);
    }
  } else if (key.find(kPrioritySmallTaskSwitchKey) != std::string::npos) {
    priority_small_task_switch_.store(nas::utils::StringToInt(value));
  } else if (key.find(kSmallTaskSizeKey) != std::string::npos) {
    small_task_size_.store(nas::utils::StringToInt(value));
  }
}

void UserInfo::OnSubscribeComplete(bool success) {
  LOG(INFO) << __func__ << ", result: " << success;
  if (!success) {
    base::ThreadPool::PostDelayedTask(
      FROM_HERE,
      base::BindOnce(&UserInfo::SubcribeRedis,
        base::Unretained(this)),
      base::Milliseconds(2000));
  }
}

void UserInfo::SubcribeRedis() {
}

void UserInfo::AddSavePath(const std::string& path) {
  // queue_.Add(path);
}

}  // namespace nas
