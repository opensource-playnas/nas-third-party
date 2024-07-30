/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2023-05-15 11:15:48
 */

#include "error_desc.h"

#include <map>

#include "nas/common/call_result.h"
#include "nas/common/error_code/general_error_code.h"

namespace nas {
const std::map<uint32_t, const char*> kDlnaErrorList = {
    {(uint32_t)DlnaServiceErrorNo::kSuccess, "成功"},
    {(uint32_t)DlnaServiceErrorNo::kParamInvalid, "参数无效"},
    {(uint32_t)DlnaServiceErrorNo::kCreateScreenCastingFailed,
     "创建投屏任务失败"},
    {(uint32_t)DlnaServiceErrorNo::kScreenCastingDeviceNotExist,
     "投屏设备不存在"},
    {(uint32_t)DlnaServiceErrorNo::kPlayUrlInvalid, "投屏链接无效"},
    {(uint32_t)DlnaServiceErrorNo::kScreenCastingDeviceNoneProtocol,
     "投屏设备支持的协议为空"},
    {(uint32_t)DlnaServiceErrorNo::kNonsupportMimeTypeExtension,
     "不支持的文件格式 mime type"},
    {(uint32_t)DlnaServiceErrorNo::kScreenCastingDeviceNonsupportProtocol,
     "投屏设备不支持该格式的文件"}};

const char* GetErrorDesc(DlnaServiceErrorNo error_no) {
  auto iter = kDlnaErrorList.find((uint32_t)error_no);
  if (iter != kDlnaErrorList.end()) {
    return iter->second;
  }
  return "未知异常";
}

}  // namespace nas