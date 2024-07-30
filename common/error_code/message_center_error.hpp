/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-08-25 17:15:15
*/

#ifndef NAS_COMMON_ERROR_CODE_MESSAGE_CENTER_ERROR_HPP_
#define NAS_COMMON_ERROR_CODE_MESSAGE_CENTER_ERROR_HPP_

#include <stdint.h>

#include "nas/common/error_code/general_error_code.h"
// 消息中心错误码
namespace nas {
  enum class MessageCenterErrorNo : uint32_t {
      // 无错误
    kSuccess = 0x0,
    // 未知错误
    kUnkown = 1,

    
    // 数据库操作失败
    kDBFailed = 0x3000,
    // 插入数据失败
    kInsertDBFailed,
    // 部分消息Id删除失败
    kDBDeletePartFailed,

    
    //请求参数无效
    kInvalidParam = 0x5000,
    // 无效登录code
    kInvalidLoginCode,
    // 无效登录凭证 
    kInvalidNasToken,
    
    // 定义错误码不能大于0xFFFF
    // 参考 ：https://thoughts.aliyun.com/workspaces/6315e060e437f0001aee8644/docs/63d5dff3e4178b00019d68a7
    kMaxLimit = 0xFFFF
  }; // MessageCenterErrorNo
} // nas
#endif //NAS_COMMON_ERROR_CODE_MESSAGE_CENTER_ERROR_HPP_