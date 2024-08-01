#ifndef NAS_AMULE_ROOT_AMULE_SRC_MULE_TASK_PUBLIC_DEFINE_H_
#define NAS_AMULE_ROOT_AMULE_SRC_MULE_TASK_PUBLIC_DEFINE_H_

#include <cstdint>
#include "../../../download_public_define.h"

class MuleDownloadTask : public DownloadTaskInterface {
public:
  virtual ~MuleDownloadTask() = default;
  virtual void SetParam(const DownloadParam&) = 0;
  virtual bool ParseLink(const char * link, ParseResult* result) = 0;
  virtual DownloadErrorCode CheckCondition(const char* source) = 0;
};

// ***********设置项***************
class AmuleSettingInterface {
public:
  virtual ~AmuleSettingInterface() = default;
  // 设置TCP监听端口
  virtual void SetTcpListenPort(int port) = 0;
  // 设置UDP监听端口
  virtual void SetUdpListenPort(int port) = 0;
  // 设置全局最大连接数
  virtual void SetMaxConnections(int connections) = 0;
  // 设置全局最大上传速度
  virtual void SetMaxUploadSpeed(int64_t speed) = 0;
  // 设置全局最大下载速度
  virtual void SetMaxDownloadSpeed(int64_t speed) = 0;
};


#endif  // NAS_AMULE_ROOT_AMULE_SRC_MULE_TASK_PUBLIC_DEFINE_H_
