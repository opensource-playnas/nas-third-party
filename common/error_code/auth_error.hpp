/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-01-31 11:15:05
*/

#ifndef NAS_COMMON_ERROR_CODE_AUTH_ERROR_HPP_
#define NAS_COMMON_ERROR_CODE_AUTH_ERROR_HPP_

#include <stdint.h>

#include "nas/common/error_code/general_error_code.h"

// 鉴权服务错误代码定义
namespace nas {
  enum class AuthServiceErrorNo : uint32_t {
    // 无错误
    kSuccess = 0x0,
    // 未知错误
    kUnkown = 1,
    // 中心服务器的token不匹配
    kMismatchCenterToken,    
    // 无效中心服务器token
    kInvalidCenterToken,
    // 中心服务器数据异常
    kCenterServiceDataException,
    // 中心服务器验证未通过
    kCenterServiceVerifyNoPass,
    // 登录用户不是设备拥有者
    kUserNoOwner,
    // 设备已经绑定,不支持重复绑定
    kDeviceBinded,
    // 设备未绑定
    kDeviceNoBind,
    // 设备绑定确认失败
    kDeviceBindAckFailed,
    // 设备换绑失败
    kDeviceBindUpdateAckFailed,
    // 用户登录信息无效
    kInvalidLoginInfo,
    // 验证码为空
    kEmptyVerifyCode,
    // 中心服务器拒绝换绑
    kCentenServiceRefuseBindUpdate,
    
    // 网络失败
    kNetworkFailed = 0x2000,

    // 数据库操作失败
    kDBFailed = 0x3000,
    // 插入数据失败
    kInsertDBFailed,
    // 数据库没找到对应的云用户id
    kDBNotFoundCloudUserId,
    // 数据库没找到用户id
    kDBNotFoundUserId,
    // 缺少必须字段
    kDBFieldLost,

    //请求参数无效
    kInvalidParam = 0x5000,
    // 无效登录code
    kInvalidLoginCode,
    // 无效登录凭证 
    kInvalidNasToken,

    // 定义错误码不能大于0xFFFF
    // 参考 ：https://thoughts.aliyun.com/workspaces/6315e060e437f0001aee8644/docs/63d5dff3e4178b00019d68a7
    kMaxLimit = 0xFFFF
  };


} // namespace nas


#endif // NAS_COMMON_ERROR_CODE_AUTH_ERROR_HPP_