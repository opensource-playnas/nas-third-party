#ifndef TORRENT_TASK_PUBLIC_DEFINE_H
#define TORRENT_TASK_PUBLIC_DEFINE_H

#include "download_public_define.h"
#include <stdint.h>

enum EncryptionMode {
    kAllow = 0, // 允许加密
    kEnforce,   // 强制加密
    kDisable    // 禁用加密
};

struct TorrenFileItemSelect {
    TorrenFileItemSelect():index(0), is_selected(0) {

    }
    int index;
    int is_selected;
};

struct TorrentDownloadParam {
    TorrentDownloadParam() {
        memset(id, 0, sizeof(id));
        memset(url, 0, sizeof(url));
        memset(save_path, 0, sizeof(save_path));
        memset(file_name, 0, sizeof(file_name));
        item_select_list = NULL;
        file_count = 0;
        is_download_now = false;
    }
    char id[kIdLength];                   // 任务id
    char url[kUrlLength];                // url链接
    char save_path[kMaxPath];      // 保存路径
    char file_name[kMaxPath];      // 文件名称
    DownloadType download_type;   // 下载类型
    TorrenFileItemSelect* item_select_list;
    int file_count;
    bool is_download_now;
};

struct TorrentDownloadProgressInfo: public DownloadProgressInfo {
    TorrentDownloadProgressInfo():DownloadProgressInfo(), file_count(0), state(-1),is_seed(false) {}
    int file_count;
    int state;
    bool is_seed;
};


typedef void (*PfnParse)(void * context, const char* result);
typedef void (*PfnTorrentAdded)(void * context);

class BtDownloadTask : public DownloadTaskInterface {
public:
    virtual ~BtDownloadTask() = default;
    // 设置参数
    virtual void SetParam(const TorrentDownloadParam& param) = 0;
    // 设置任务添加回调
    virtual void SetTorrentAddedCallback(void* context, PfnTorrentAdded callback) = 0;
    // 获取种子文件列表数量
    virtual int GetTorrentFileCount() = 0;
    // 根据索引获取文件信息
    virtual FileBaseInfo GetTorrentFileInfoByIndex(int index) = 0;
    // 获取种子文件基本信息
    virtual FileBaseInfo GetTorrentFileBaseInfo() = 0;
    // 检查资源文件
    virtual DownloadErrorCode CheckCondition(const char * source) = 0;
    // 检查资源是否有效
    virtual bool IsValid(const char * source) = 0;
    // 获取做种信息
    virtual char * GetSeedingInfo() = 0;
    // 获取种子列表，返回json
    virtual char * GetTorrentList() = 0;
    // 获取种子分享率
    virtual int GetTorrentShareRatio() = 0;
    // 获取种子做种时间
    virtual int64_t GetSeedingTime() = 0;
    // 获取上传速度
    virtual int GetUploadRate() = 0;
    // 是否校验中
    virtual bool IsChecking() = 0;
    // 是否下载中
    virtual bool IsDownloading() = 0;
};


// ***********设置项***************
class BtSettingInterface {
public:
    virtual ~BtSettingInterface() = default;
    // 是否创建子文件夹
    virtual void EnableCreateSubdir(bool enable) = 0;
    // 是否自动下载
    virtual void EnableAutoDownload(bool enable) = 0;
    // 设置TCP监听端口
    virtual void SetTcpListenPort(int port) = 0;
    // 设置最大下载速度
    virtual void SetMaxDownloadSpeed(int64_t speed) = 0;
    // 设置最大上传速度
    virtual void SetMaxUploadSpeed(int64_t speed) = 0;
    // 设置加密模式
    virtual void SetEncryptionMode(EncryptionMode mode) = 0;
    // 设置全局最大连接数
    virtual void SetMaxConnections(int connections) = 0;
    // 设置每个torrent最大连接数
    virtual void SetTorrentMaxConnections(int connections) = 0;
    // 是否启动DHT网络
    virtual void EnableDHT(bool enable) = 0;
    // 是否启用 UPnP/NAT-PMP端口转发（需要路由器支持）
    virtual void EnableUPnPAndNat(bool enable) = 0;
    // 是否为所有文件预分配磁盘空间
    virtual void SetPreallocationEnabled(bool enable) = 0;
};

class TorrentFileDownloadTask : public BtDownloadTask {
public:
    virtual ~TorrentFileDownloadTask() = default;
    // 种子文件转磁力链接
    virtual int CreateMagnetUri(char * buf) = 0;
};

class MagnetDownloadTask : public BtDownloadTask {
public:
    virtual ~MagnetDownloadTask() = default;
    // 设置解析回调
    virtual void SetParseCallback(void* context, PfnParse call_back) = 0;
};

#endif // TORRENT_TASK_PUBLIC_DEFINE_H
