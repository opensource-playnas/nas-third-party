/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-09-26 14:55:37
 */

#ifndef NAS_COMMON_OSS_OSS_MANAGER_H
#define NAS_COMMON_OSS_OSS_MANAGER_H


#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <string>


namespace nas {
using UploadCallback =
std::function<void(size_t increment, int64_t transfered, int64_t total)>;

class OssMgr {
 public:
 OssMgr();
 ~OssMgr();
  void SetOssInfo(const std::string& access_key_id,
                  const std::string& access_key_secret,
                  const std::string& endpoint,
                  const std::string& security_token);

  bool Upload(const std::string& bucket_name,
              const std::string& relative_path_file /*utf-8 encode*/,
              const std::string& file_path /*gb2312 encode*/);

 private:
  std::string access_key_id_;
  std::string access_key_secret_;
  std::string endpoint_;
  std::string security_token_;
};

};  // namespace nas

#endif