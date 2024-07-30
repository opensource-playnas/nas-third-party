/*
* @Description: 
* @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
* @Author: fengbangyao@ludashi.com
* @Date: 2023-02-13 11:32:00
*/

#ifndef NAS_COMMON_ERROR_CODE_GATEWAY_ERROR_HPP_
#define NAS_COMMON_ERROR_CODE_GATEWAY_ERROR_HPP_

#include <stdint.h>

namespace nas {
  enum class GatewayErrorNo : uint32_t {
    // 成功
    kSuccess = 0x0,
    // 未知异常
    kUnkown,
    // 未发现调用的api
    kNotFoundApi,
    // 调用服务错误,请检查服务是否正常
    kCallServiceError,
    // 目标服务已响应,但数据处理错误
    kProcessResponseExcepion,
    // 设备不匹配
    kNotMatchDevice,
    // 获取设备信息失败
    kGetDeviceInfoError,

    // 配置错误
    kStorageError = 0x100,
    // 一个无效的json字符串
    kStorageInvalidJson,
    // 配置指定namespace为空
    kStorageNamespaceEmpty,
    // 配置指定field为空
    kStorageKeyEmpty,
    // 未指定value字段
    kStorageValueEmpty,
    // 解析user_id失败
    kStorageUserIdEmpty,
    // 设置失败
    kStorageSetFailed

  }; // enum class GatewayErrorNo

} // namespace nas

#endif // NAS_COMMON_ERROR_CODE_GATEWAY_ERROR_HPP_