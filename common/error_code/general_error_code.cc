/*
 * @Description:
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: fengbangyao@ludashi.com
 * @Date: 2023-01-31 10:25:20
 */

#include "nas/common/error_code/general_error_code.h"

#include "base/strings/stringprintf.h"

#include "base/check.h"
namespace nas {

ErrorHandle::ErrorHandle(ErrorSource error_source) {
  error_source_ = error_source;
}

ErrorSource ErrorHandle::GetErrorSource() {
  return error_source_;
}

void ErrorHandle::SetErrorNo(uint32_t err_no) {
  if ((err_no != kRequestSuccess) && !HasErrorSource(err_no)) {
    // 调用模块必须设置error_source
    DCHECK(error_source_ > ErrorSource::kOk);
    uint32_t source = static_cast<uint32_t>(error_source_);
    final_error_no_ = ((source << 16) & kErrorSourceMaskCode) + err_no;
  } else {
    final_error_no_ = err_no;
  }
}

uint32_t ErrorHandle::GetErrorNo() {
  return final_error_no_;
}

bool ErrorHandle::HasErrorSource(uint32_t error_no) {
  DCHECK(kErrorSourceMaskCode != error_no);
  return (error_no & kErrorSourceMaskCode) > 0;
}

void ErrorHandle::SetErrorMsg(const std::string& msg) {
  module_msg_desc_ = msg;
}

std::string ErrorHandle::GetErrorMsg() {
  return module_msg_desc_;
}
std::string FormatErrorDescTable(
    ErrorSource error_source,
    const std::map<uint32_t, const char*> error_list) {
  std::string table_header = "| 错误码 (十进制) | 错误码 (十六进制) | 描述 |\n";
  std::string table_row = "|----------------|----------------|------|\n";
  for (const auto& [error_code, msg] : error_list) {
    ErrorHandle error_handle(error_source);
    error_handle.SetErrorNo((uint32_t)error_code);
    int final_error_no = error_handle.GetErrorNo();

    std::string str_value;
    base::SStringPrintf(&str_value, "| %d | 0x%0X | %s |\n", final_error_no,
                        final_error_no, msg);
    table_row += str_value;
  }

  return table_header + table_row;
}

}  // namespace nas