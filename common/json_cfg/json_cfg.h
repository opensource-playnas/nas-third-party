/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-26 19:41:57
 */

#ifndef NAS_COMMON_JSON_CFG_JSON_CFG_H_
#define NAS_COMMON_JSON_CFG_JSON_CFG_H_

#include <string>

#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"

/*
 TODO:未测试多线程多进程读写问题
*/

namespace nas {
class JsonFile {
 public:
  JsonFile(const base::FilePath& file_path);

  bool GetInt(const std::string& key,
              int32_t* value,
              int32_t default_value,
              bool use_default);
  bool GetIntByPath(const std::string& key,
              int32_t* value,
              int32_t default_value,
              bool use_default);

  bool GetBool(const std::string& key,
              bool* value,
              bool default_value,
              bool use_default);

  bool GetString(const std::string& key,
                 std::string* value,
                 const std::string& default_value,
                 bool use_default);

  bool GetStringByPath(const std::string& path,
                 std::string* value,
                 const std::string& default_value,
                 bool use_default);

  bool GetValue(const std::string& key,
                base::Value* value,
                const base::Value& default_value,
                bool use_default);

  void SetInt(const std::string& key, int32_t value);

  void SetString(const std::string& key, const std::string& value);

  void SetValue(const std::string& key, const base::Value& value);

  bool WriteFile();

 private:
  base::FilePath file_path_;

  // 文件根节点一定是dict
  absl::optional<base::Value> root_;
};  // JsonFile

void NasConfigSetString(const base::FilePath& file_name,
                               const std::string& key,
                               const std::string& value);
void NasConfigSetInt(const base::FilePath& file_name,
                            const std::string& key,
                            int32_t value);

bool NasConfigGetString(const base::FilePath& file_name,
                               const std::string& key,
                               std::string* value,
                               const std::string& default_value,
                               bool use_default);
bool NasConfigGetStringByPath(const base::FilePath& file_name,
                        const std::string& path,
                        std::string* value,
                        const std::string& default_value,
                        bool use_default);

bool NasConfigGetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t* value,
                     int32_t default_value,
                     bool use_default);

bool NasConfigGetIntByPath(const base::FilePath& file_name,
                     const std::string& path,
                     int32_t* value,
                     int32_t default_value,
                     bool use_default);

bool NasConfigGetBool(const base::FilePath& file_name,
                     const std::string& key,
                     bool* value,
                     bool default_value,
                     bool use_default);

}  // namespace nas

#endif  // NAS_COMMON_JSON_CFG_JSON_CFG_H_