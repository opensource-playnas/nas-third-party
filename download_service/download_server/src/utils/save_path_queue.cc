/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-07-14 10:25:07
 */

#include "save_path_queue.h"

#include "base/values.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"

namespace nas {
static const char* kNameSpace = "download";
static const char* kSavePathKey = "download_dir";

SavePathQueue::SavePathQueue(const std::string& user_id, size_t max_size)
    : user_id_(user_id), max_size_(max_size) {
  Init();
}

void SavePathQueue::Init() {
  base::Value::Dict root_dict;
  base::Value* data = root_dict.Find(kNameSpace);
  if (data) {
    base::Value::Dict* data_dict = data->GetIfDict();
    if (data_dict) {
      std::string* save_path = data_dict->FindString(kSavePathKey);
      if (save_path) {
        std::vector<std::string> folders = base::SplitString(
          *save_path, "|", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
        for (const auto& folder : folders) {
          Add(folder, true);
        }
      }
    }
  }
  
}

void SavePathQueue::Add(const std::string& element, bool is_init /*= false*/) {
  // 如果元素已经存在，先删除旧的元素
  if (lookup_table_.count(element) > 0) {
    queue_.erase(lookup_table_[element]);
  }

  // 如果队列已满，删除队列头部的元素
  if (queue_.size() == max_size_) {
    lookup_table_.erase(queue_.front());
    queue_.pop_front();
  }

  // 插入新元素到队列尾部，并更新查找表
  queue_.push_back(element);
  lookup_table_[element] = std::prev(queue_.end());

  if (!is_init){
    // 存储到redis
    std::string result = GetElementsString();
  }
}

std::string SavePathQueue::GetElementsString() {
  std::string elements_string;
  for (auto it = queue_.begin(); it != queue_.end(); ++it) {
    elements_string += *it;
    if (std::next(it) != queue_.end()) {
      elements_string += " | ";
    }
  }
  return elements_string;
}

}  // namespace nas
