// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/08/21 15:28

#include "list_screen_casting_devices.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/escape.h"
#include "base/strings/string_split.h"
#include "path/file_path_unit.h"
namespace nas {

void ListScreenCastingDevicesCallDate::Process() {
  screen_casting_mgr_->GetMediaRenderesList(
      &response_,
      base::BindOnce(&ListScreenCastingDevicesCallDate::ProcessComplete,
                     base::Unretained(this)));
}

void ListScreenCastingDevicesCallDate::ProcessComplete() {
  OnProcessed();
}

}  // namespace nas
