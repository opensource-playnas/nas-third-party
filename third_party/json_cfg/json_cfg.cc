/*
 * @Description:
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-01-31 15:43:20
 */

#include "json_cfg.h"

#include <base/json/json_parser.h>

#include "../file/file_utils.h"


namespace nas {
JsonFile::JsonFile(const base::FilePath& file_path) {
  file_path_ = file_path;

  std::string content;
  if (file_utils::ReadFileToStrBuf(file_path_.value().c_str(), &content)) {
    root_.reset(base::JSONReader::Read(content));
  }

  if (!root_) {
    root_.reset(base::JSONReader::Read("{}"));
  }
}

bool JsonFile::GetInt(const std::string& key,
                      int32_t* value,
                      int32_t default_value) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::DictionaryValue* directory_root = nullptr;
  root_->GetAsDictionary(&directory_root);

  if (directory_root) {
    if (!directory_root->GetInteger(key, value)) {
      *value = default_value;
      result = true;
    }
  }

  return result;
}

bool JsonFile::GetBool(const std::string& key,
                       bool* value,
                       bool default_value) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::DictionaryValue* directory_root = nullptr;
  root_->GetAsDictionary(&directory_root);

  if (directory_root) {
    if (!directory_root->GetBoolean(key, value)) {
      *value = default_value;
      result = true;
    }
  }

  return result;
}

bool JsonFile::GetString(const std::string& key,
                         std::string* value,
                         const std::string& default_value) {
  DCHECK(value && root_);
  if (!value) {
    return false;
  }

  bool result = false;
  base::DictionaryValue* directory_root = nullptr;
  root_->GetAsDictionary(&directory_root);

  if (directory_root) {
    if (!directory_root->GetString(key, value)) {
      *value = default_value;
      result = true;
    }
  }

  return result;
}

bool JsonFile::GetValue(const std::string& key,
                        base::Value* value,
                        const base::Value& default_value) {
  DCHECK(value && root_);
  DCHECK(false && "todo:");
  return true;
}

bool JsonFile::SetInt(const std::string& key, int32_t value) {
  DCHECK(root_);

  base::DictionaryValue* directory_root = nullptr;
  root_->GetAsDictionary(&directory_root);

  if (directory_root) {
    directory_root->SetInteger(key, value);
    return true;
  } else {
    return false;
  }
}

bool JsonFile::SetBool(const std::string& key, bool value) {
  DCHECK(root_);

  base::DictionaryValue* directory_root = nullptr;
  root_->GetAsDictionary(&directory_root);

  if (directory_root) {
    directory_root->SetBoolean(key, value);
    return true;
  } else {
    return false;
  }
}

bool JsonFile::SetString(const std::string& key, const std::string& value) {
  DCHECK(root_);
  base::DictionaryValue* directory_root = nullptr;
  root_->GetAsDictionary(&directory_root);

  if (directory_root) {
    directory_root->SetString(key, value);
    return true;
  } else {
    return false;
  }
}

bool JsonFile::SetValue(const std::string& key, const base::Value& value) {
  DCHECK(false && "todo:");

  return false;
}

bool JsonFile::WriteFile() {
  std::string content;
  bool result = false;
  if (root_) {
    base::JSONWriter::Write(root_.get(), &content);
    if (!content.empty()) {
      result = file_utils::WriteFileFromBuf(
          file_path_.value().c_str(), (LPBYTE)content.c_str(),
                                   content.size());
    }
  }

  return result;
}

bool NasConfigSetString(const base::FilePath& file_name,
                        const std::string& key,
                        const std::string& value) {
  bool ret = false;
  JsonFile json_file(file_name);
  if (json_file.SetString(key, value)) {
    ret = json_file.WriteFile();
  }
  return ret;
}
bool NasConfigSetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t value) {
  bool ret = false;
  JsonFile json_file(file_name);

  if (json_file.SetInt(key, value)) {
    ret = json_file.WriteFile();
  }

  return ret;
}

bool NasConfigSetBool(const base::FilePath& file_name,
                      const std::string& key,
                      bool value) {
  bool ret = false;
  JsonFile json_file(file_name);

  if (json_file.SetBool(key, value)) {
    ret = json_file.WriteFile();
  }

  return ret;
}

void NasConfigGetString(const base::FilePath& file_name,
                        const std::string& key,
                        std::string* value,
                        const std::string& default_value) {
  JsonFile json_file(file_name);
  json_file.GetString(key, value, default_value);
}

void NasConfigGetInt(const base::FilePath& file_name,
                     const std::string& key,
                     int32_t* value,
                     int32_t default_value) {
  JsonFile json_file(file_name);
  json_file.GetInt(key, value, default_value);
}


void NasConfigGetBool(const base::FilePath& file_name,
                      const std::string& key,
                      bool* value,
                      bool default_value) {
  JsonFile json_file(file_name);
  json_file.GetBool(key, value, default_value);
}

}  // namespace nas