/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-07-14 10:25:07
 */
#ifndef NAS_DOWNLOAD_SERVER_SRC_UTILS_THREAD_SAFE_QUEUE_H_
#define NAS_DOWNLOAD_SERVER_SRC_UTILS_THREAD_SAFE_QUEUE_H_

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>

namespace nas {
class SavePathQueue {
 public:
  explicit SavePathQueue(const std::string& user_id, size_t max_size);
  void Add(const std::string& element, bool is_init = false);

 private:
  void Init();
  std::string GetElementsString();

 private:
  std::string user_id_;
  size_t max_size_;

  std::list<std::string> queue_;
  std::unordered_map<std::string, std::list<std::string>::iterator>
      lookup_table_;
};
}  // namespace nas

#endif  // NAS_DOWNLOAD_SERVER_SRC_UTILS_THREAD_SAFE_QUEUE_H_