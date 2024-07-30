/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2022-11-26 19:48:54
 */

#include "base/check.h"
#include "base/files/file_path.h"
#include "base/logging.h"

#include "nas/common/json_cfg/json_cfg.h"

namespace nas {
JsonFile::JsonFile(const base::FilePath& file_path) {
  file_path_ = file_path;

  std::string content;
  if (base::ReadFileToString(file_path_, &content)) {
    root_ = base::JSONReader::Read(content);
  } else {
    LOG(ERROR) << "read file failed:" << file_path;
  }

  if (!root_.has_value()) {
    root_ = base::JSONReader::Read("{}");
  }
}

bool JsonFile::GetInt(const std::string& key,
                      int32_t* value,
                      int32_t default_value,
                      bool use_default) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    absl::optional<int> json_value = root_dict->FindInt(key);
    if (json_value.has_value()) {
      *value = json_value.value();
      result = true;
    }
  }

  if (!result && use_default) {
    *value = default_value;
    result = true;
  }

  return result;
}

bool JsonFile::GetIntByPath(const std::string& key,
                            int32_t* value,
                            int32_t default_value,
                            bool use_default) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    absl::optional<int> json_value = root_dict->FindIntByDottedPath(key);
    if (json_value.has_value()) {
      *value = json_value.value();
      result = true;
    }
  }

  if (!result && use_default) {
    *value = default_value;
    result = true;
  }

  return result;
}

bool JsonFile::GetBool(const std::string& key,
                       bool* value,
                       bool default_value,
                       bool use_default) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    absl::optional<int> json_value = root_dict->FindBool(key);
    if (json_value.has_value()) {
      *value = json_value.value();
      result = true;
    }
  }

  if (!result && use_default) {
    *value = default_value;
    result = true;
  }

  return result;
}

bool JsonFile::GetString(const std::string& key,
                         std::string* value,
                         const std::string& default_value,
                         bool use_default) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    std::string* json_value = root_dict->FindString(key);
    if (json_value) {
      *value = *json_value;
      result = true;
    }
  }

  if (!result && use_default) {
    *value = default_value;
    result = true;
  }

  return result;
}

bool JsonFile::GetStringByPath(const std::string& path,
                               std::string* value,
                               const std::string& default_value,
                               bool use_default) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    std::string* json_value = root_dict->FindStringByDottedPath(path);
    if (json_value) {
      *value = *json_value;
      result = true;
    }
  }

  if (!result && use_default) {
    *value = default_value;
    result = true;
  }

  return result;
}

bool JsonFile::GetValue(const std::string& key,
                        base::Value* value,
                        const base::Value& default_value,
                        bool use_default) {
  DCHECK(value && root_);
  DCHECK(false && "todo:");
  return true;
}

void JsonFile::SetInt(const std::string& key, int32_t value) {
  DCHECK(root_);
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    root_dict->Set(key, value);
  }
}

void JsonFile::SetString(const std::string& key, const std::string& value) {
  DCHECK(root_);
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    root_dict->Set(key, value);
  }
}

void JsonFile::SetValue(const std::string& key, const base::Value& value) {
  DCHECK(false && "todo:");
}

bool JsonFile::WriteFile() {
  std::string content;
  bool result = false;
  base::Value::Dict* root_dict = root_->GetIfDict();
  if (root_dict) {
    base::JSONWriter::Write(*root_dict, &content);
    if (!content.empty()) {
      result = base::WriteFile(file_path_, content);
    }
  }

  return result;
}

void NasConfigSetString(const base::FilePath& file_name,
                        const std::string& key,
                        const std::string& value) {
  JsonFile json_file(file_name);
  json_file.SetString(key, value);
  if (!json_file.WriteFile()) {
    LOG(INFO) << "write json file failed, key: " << key << ", value: " << value;
  }
}
void NasConfigSetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t value) {
  JsonFile json_file(file_name);
  json_file.SetInt(key, value);
  json_file.WriteFile();
}

bool NasConfigGetString(const base::FilePath& file_name,
                        const std::string& key,
                        std::string* value,
                        const std::string& default_value,
                        bool use_default) {
  JsonFile json_file(file_name);
  return json_file.GetString(key, value, default_value, use_default);
}

bool NasConfigGetStringByPath(const base::FilePath& file_name,
                              const std::string& path,
                              std::string* value,
                              const std::string& default_value,
                              bool use_default) {
  JsonFile json_file(file_name);
  return json_file.GetStringByPath(path, value, default_value, use_default);
}

bool NasConfigGetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t* value,
                     int32_t default_value,
                     bool use_default) {
  JsonFile json_file(file_name);
  return json_file.GetInt(key, value, default_value, use_default);
}

bool NasConfigGetIntByPath(const base::FilePath& file_name,
                           const std::string& path,
                           int32_t* value,
                           int32_t default_value,
                           bool use_default) {
  JsonFile json_file(file_name);
  return json_file.GetIntByPath(path, value, default_value, use_default);
}

bool NasConfigGetBool(const base::FilePath& file_name,
                      const std::string& key,
                      bool* value,
                      bool default_value,
                      bool use_default) {
  JsonFile json_file(file_name);
  return json_file.GetBool(key, value, default_value, use_default);
}

}  // namespace nas