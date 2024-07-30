/*
 * @Description: service start error code
 * @copyright 2022 The Master Lu PC-Group Authors. All rights reserved
 * @Author: wanyan@ludashi.com
 * @Date: 2022-10-11 19:22:01
 */

#ifndef GENERAL_ERROR_CODE_H_
#define GENERAL_ERROR_CODE_H_

#include <memory>
#include <string>
#include <map>
#include "nas/common/error_code/auth_error.hpp"
#include "nas/common/error_code/gateway_error.hpp"

namespace nas{
  // 默认请求成功错误码
  const int kRequestSuccess = 0;
  // 错误源开始表示的位数起始值
  const uint32_t kErrorSourceMaskCode = 0xFFFF0000;
  // 错误码上报的模块
  enum class ErrorSource {
    // 无错误
    kOk = 0,
    // 动态网关
    kGateway = 0x10,
    // 服务管理
    kServicesManager,
    // 鉴权服务
    kAuthService,
    // 文件服务
    kFileService,
    // 消息中心
    kMessageCenter,
    // 媒体服务器
    kMediaService,
    // 图片服务
    kImageService,
    // 远程下载服务
    kDownloadService,
    // 更新服务
    kUpdateService,
    // 系统信息服务
    kSystemService,
    // grpc_proxy服务
    kGrpcProxyService,
    // Dlna服务器
    kDlnaService,
  };

/*
 * 错误处理单实列,每个模块内部定义一个,避免将错误代码层层传递，
 * 以及方便模块对外提供自身错误信息
 */
class ErrorHandle {
 public:
  ErrorHandle(ErrorSource source_code);
  // 获取错误来源编号
  ErrorSource GetErrorSource();
  // 设置错误编号,各个模块定义错误方式不同,统一使用uint32_t类型接收
  void SetErrorNo(uint32_t module_err_no);
  uint32_t GetErrorNo();
  // 设置模块对应的消息描述
  void SetErrorMsg(const std::string& msg);
  // 获取消息描述
  std::string GetErrorMsg();

 private:
  // 判断错误码是否已经带了来源
  bool HasErrorSource(uint32_t error_no);

 private:
  // 包含错误来源 + 错误原因
  uint32_t final_error_no_ = kRequestSuccess;
  // 错误来源
  ErrorSource error_source_ = ErrorSource::kOk;
  // 错误消息描述
  std::string module_msg_desc_;
};  // class ErrorHandle

std::string FormatErrorDescTable(
    ErrorSource error_source,
    const std::map<uint32_t, const char*> error_list);

}  // namespace nas

#endif
