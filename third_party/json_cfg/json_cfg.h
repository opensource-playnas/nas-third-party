/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-01-31 15:43:20
 */

#ifndef NAS_MSVC_UNIT_JSON_CFG_JSON_CFG_H_
#define NAS_MSVC_UNIT_JSON_CFG_JSON_CFG_H_

#include <string>

#include <chromium/chromium_base_helper.h>

#include <base/values.h>
#include <base/files/file_util.h>
#include <base/json/json_reader.h>
#include <base/json/json_writer.h>
#include <base/memory/scoped_ptr.h>

/*
 TODO:未测试多线程多进程读写问题
*/

namespace nas {
class JsonFile {
 public:
  JsonFile(const base::FilePath& file_path);

  bool GetInt(const std::string& key, int32_t* value, int32_t default_value);

  bool GetBool(const std::string& key, bool* value, bool default_value);

  bool GetString(const std::string& key,
                 std::string* value,
                 const std::string& default_value);

  bool GetValue(const std::string& key,
                base::Value* value,
                const base::Value& default_value);

  bool SetInt(const std::string& key, int32_t value);

  bool SetBool(const std::string& key, bool value);

  bool SetString(const std::string& key, const std::string& value);

  bool SetValue(const std::string& key, const base::Value& value);

  bool WriteFile();

 private:
  base::FilePath file_path_;

  // 文件根节点一定是dict
  scoped_ptr<base::Value> root_;
};  // JsonFile

bool NasConfigSetString(const base::FilePath& file_name,
                        const std::string& key,
                        const std::string& value);

bool NasConfigSetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t value);

bool NasConfigSetBool(const base::FilePath& file_name,
                     const std::string& key,
                     bool value);

void NasConfigGetString(const base::FilePath& file_name,
                        const std::string& key,
                        std::string* value,
                        const std::string& default_value);

void NasConfigGetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t* value,
                     int32_t default_value);

void NasConfigGetBool(const base::FilePath& file_name,
                     const std::string& key,
                     bool* value,
                     bool default_value);

}  // namespace nas

#endif  // NAS_MSVC_UNIT_JSON_CFG_JSON_CFG_H_