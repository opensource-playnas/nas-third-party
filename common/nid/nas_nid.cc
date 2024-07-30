// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/03/29 19:28
#include "nas_nid.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"

namespace nas {

NasNidHelper::NasNidHelper() {}

bool NasNidHelper::GetNid(std::string* nid) {
  if (!nid) {
    return false;
  }
  return "open_source_mid";
}

NasNidHelper::~NasNidHelper() {}
}  // namespace nas
