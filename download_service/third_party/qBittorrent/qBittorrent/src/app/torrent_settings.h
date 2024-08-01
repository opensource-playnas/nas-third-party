#ifndef TORRENTSETTINGS_H
#define TORRENTSETTINGS_H

#include "torrent_task_public_define.h"

class TorrentSettings: public BtSettingInterface
{
public:
    TorrentSettings();
    ~TorrentSettings();

public:
    // 是否创建子文件夹
   void EnableCreateSubdir(bool enable) override;
   // 是否自动下载
   void EnableAutoDownload(bool enable) override;
   // 设置TCP监听端口
   void SetTcpListenPort(int port) override;
   // 设置最大上传速度
   void SetMaxUploadSpeed(int64_t speed) override;
   // 设置最大下载速度
   void SetMaxDownloadSpeed(int64_t speed) override;
   // 设置加密模式
   void SetEncryptionMode(EncryptionMode mode) override;
   // 设置全局最大连接数
   void SetMaxConnections(int connections) override;
   // 设置每个torrent最大连接数
   void SetTorrentMaxConnections(int connections) override;
   // 启动DHT网络
   void EnableDHT(bool enable) override;
   // 是否启用 UPnP/NAT-PMP端口转发（需要路由器支持）
   void EnableUPnPAndNat(bool enable) override;
   // 是否为所有文件预分配磁盘空间
   void SetPreallocationEnabled(bool enable) override;
};

#endif // TORRENTSETTINGS_H
