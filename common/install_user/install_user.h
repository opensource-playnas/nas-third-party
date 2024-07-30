/*
 * @Description: 封装了获取安装了nas的系统用户相关信息；如sid；访问令牌
 * @copyright 2023 The Master Lu PC-Group Authors. All rights reserved
 * @Author: guopengwei@ludashi.com
 * @Date: 2023-09-13 13:37:52
 */

#include "nas/common/nas_config.h"

#include <string>

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#endif  // BUILDFLAG(IS_WIN)

#ifndef NAS_COMMON_INSTALL_USER_INSTALL_USER_H
#define NAS_COMMON_INSTALL_USER_INSTALL_USER_H

namespace nas {

std::string GetSid(unsigned short level);

class InstallUserToken {
 public:
  explicit InstallUserToken(unsigned short level);
  ~InstallUserToken();

#if BUILDFLAG(IS_WIN)
  // 对外提供的函数，返回句柄
  HANDLE GetToken() const;
#endif  // BUILDFLAG(IS_WIN)

 private:
#if BUILDFLAG(IS_WIN)
  void OpenToken(unsigned short level);
  HANDLE token_;
#endif  // BUILDFLAG(IS_WIN)
};

};  // namespace nas

#endif  // NAS_COMMON_INSTALL_USER_INSTALL_USER_H