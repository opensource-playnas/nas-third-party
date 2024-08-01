#ifndef __AMULE_SETTINGS_H
#define __AMULE_SETTINGS_H

#include "mule_task_public_define.h"

class AmuleSettings: public AmuleSettingInterface
{
public:
    AmuleSettings();
    ~AmuleSettings();

public:
   // 设置TCP监听端口
   void SetTcpListenPort(int port) override;
   // 设置UDP监听端口
   void SetUdpListenPort(int port) override;
   // 设置全局最大连接数
   void SetMaxConnections(int connections) override;
   // 设置全局最大上传速度
   void SetMaxUploadSpeed(int64_t speed) override;
   // 设置全局最大下载速度
   void SetMaxDownloadSpeed(int64_t speed) override;
};

#endif // __AMULE_SETTINGS_H
