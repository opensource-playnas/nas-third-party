#ifndef TORRENT_HELPER_H
#define TORRENT_HELPER_H

#include <set>
#include "base/bittorrent/torrent.h"
#include "base/bittorrent/infohash.h"
#include "base/bittorrent/torrentinfo.h"
#include "base/bittorrent/magneturi.h"
#include "base/bittorrent/session.h"
#include "base/bittorrent/peerinfo.h"

#include "torrent_task_public_define.h"

namespace Utils {
enum FileType : int64_t {
  KUnknown = 0,
  kAll = 1,             // 所有类型
  kUndefine = 1 << 1,   // 未定义类型
  kDriver = 1 << 2,     // 驱动器
  kDirectory = 1 << 3, // 目录
  kTxt = 1 << 4,  // 文档 txt
  kPdf = 1 << 5,   // 文档 pdf
  kWord = 1 << 6,      // 文档 word
  kPPT= 1 << 7,      // 文档 ppt
  kXlxs= 1 << 8,      // 文档 xlxs
  kHtml = 1 << 9, // html
  kCompress = 1 << 10,   // 压缩文件
  kPicture = 1 << 11,    // 图片
  kVideo = 1 << 12,    // 视频
  kMusic = 1 << 13, // 音乐
  kApk = 1 << 14, // apk
  kExe = 1 << 15, // exe
  kBt = 1 << 16, // bt
};
}

// Overview info
static const QString kTaskIdKey = QString::fromUtf8("task_id");
static const QString kTaskNameKey = QString::fromUtf8("task_name");
static const QString kPrivateKey = QString::fromUtf8("is_private");
static const QString kShareRatioKey = QString::fromUtf8("share_ratio");
static const QString kDownloadedSizeKey = QString::fromUtf8("downloaded_size");
static const QString kUploadedSizeKey = QString::fromUtf8("uploaded_size");
static const QString kActiveSeedsCountKey = QString::fromUtf8("active_seeds_count");
static const QString kTotalSeedsCountKey = QString::fromUtf8("total_seeds_count");
static const QString kActivePeerCountKey = QString::fromUtf8("active_peer_count");
static const QString kTotalPeerCountKey = QString::fromUtf8("total_peer_count");
static const QString kSeedingTimeKey = QString::fromUtf8("seeding_time");
static const QString kConnectionsCountKey = QString::fromUtf8("connections_count");
static const QString kMaxConnectionsCountKey = QString::fromUtf8("max_connections_count");
static const QString kDownloadSpeedKey = QString::fromUtf8("download_speed");
static const QString kUploadSpeedKey = QString::fromUtf8("upload_speed");
static const QString kDownloadAverageSpeedKey = QString::fromUtf8("download_average_speed");
static const QString kUploadAverageSpeedKey = QString::fromUtf8("upload_average_speed");
static const QString kCreatorKey = QString::fromUtf8("creator");
static const QString kCreateTimeKey = QString::fromUtf8("create_time");
static const QString kTotalSizeKey = QString::fromUtf8("total_size");
static const QString kTorrentSizeKey = QString::fromUtf8("torrent_size");
static const QString kHashV1Key = QString::fromUtf8("hash_v1");
static const QString kHashV2Key = QString::fromUtf8("hash_v2");
static const QString kSavePathKey = QString::fromUtf8("save_path");
static const QString kCommentKey = QString::fromUtf8("comment");

// tracker list
static const QString kTrackers = QString::fromUtf8("trackers");
static const QString kTierKey = QString::fromUtf8("tier");
static const QString kUrlKey = QString::fromUtf8("url");
static const QString kStatusKey = QString::fromUtf8("status");
static const QString kPeersKey = QString::fromUtf8("peers");
static const QString kSeedsKey = QString::fromUtf8("seeds");
//static const QString kLeechesKey = QString::fromUtf8("leeches");
//static const QString kDownloadedNumKey = QString::fromUtf8("downloaded_num");
static const QString kMsgKey = QString::fromUtf8("msg");

// peers list
static const QString kPeers = QString::fromUtf8("peers");
static const QString kCountryKey = QString::fromUtf8("country");
static const QString kIpKey = QString::fromUtf8("ip");
static const QString kPortKey = QString::fromUtf8("port");
static const QString kConnectionTypeKey = QString::fromUtf8("connection_type");
static const QString kDownloadProgressKey = QString::fromUtf8("download_progress");
static const QString kTotalDownloadSizeKey = QString::fromUtf8("total_download_size");
static const QString kTotalUploadSizeKey = QString::fromUtf8("total_upload_size");

// other
static const QString kTaskSizeKey = QString::fromUtf8("task_size");
static const QString kTaskProgressKey = QString::fromUtf8("task_progress");
static const QString kSeparatorKey = QString::fromUtf8("/");
static const QString kChildrenKey = QString::fromUtf8("children");
static const QString kChildNameKey = QString::fromUtf8("name");
static const QString kChildIndexKey = QString::fromUtf8("index");
static const QString kChildSizeKey = QString::fromUtf8("size");
static const QString kChildTypeKey = QString::fromUtf8("type");
static const QString kChildSelectedKey = QString::fromUtf8("selected");
static const QString kChildProgressKey = QString::fromUtf8("progress");

static const QString kNA = QString::fromUtf8("N/A");
static const QString kWorking = QString::fromUtf8("Working");;
static const QString kDisabled = QString::fromUtf8("Disabled");
static const QString kUpdating = QString::fromUtf8("Updating...");
static const QString kNotWorking = QString::fromUtf8("Not working");
static const QString kNotContacted = QString::fromUtf8("Not contacted yet");

static const QString kEmpty = QString::fromUtf8("");
static const QString kDHT = QString::fromUtf8("** [DHT] **");
static const QString kPeX = QString::fromUtf8("** [PeX] **");
static const QString kLSD = QString::fromUtf8("** [LSD] **");
static const QString kPrivate = QString::fromUtf8("This torrent is private");;

struct TorrentParam {
    QString id;
    QString url;
    Path save_path;
    QString file_name;
    DownloadType download_type;
    TorrenFileItemSelect * item_select_list;
    int file_count;
    bool is_download_now;
};

class TorrentHelper
{
public:
    TorrentHelper();
    // 转换参数
    TorrentParam ConvertParam(const TorrentDownloadParam& param);

    // 添加Magnet
    bool AddMagnet();
    // 添加种子
    bool AddTorrent(int file_count, BitTorrent::TorrentInfo & torrent_info);

    // 控制相关
    bool Pause(BitTorrent::Torrent* torrent_handle);
    bool Resume(BitTorrent::Torrent* torrent_handle, bool is_failed);

    // 做种相关
    char * GetSeedingInfo(BitTorrent::Torrent* torrent_handle);
    QJsonArray GetPeersList(BitTorrent::Torrent* torrent_handle);
    QJsonArray GetTrackerList(BitTorrent::Torrent* torrent_handle);
    void LoadStickyItems(QJsonArray* json_array, BitTorrent::Torrent* torrent_handle);
    void LoadActualItems(QJsonArray* json_array, BitTorrent::Torrent* torrent_handle);
    void ReleaseBuf(const char * buf);

    // 进度相关
    void GetProgressInfo(BitTorrent::Torrent* torrent_handle, TorrentDownloadProgressInfo** info);
    int GetSelectedTorrentFileCount(BitTorrent::Torrent* torrent_handle);
    FileBaseInfo GetFileBaseInfo(const BitTorrent::TorrentInfo& torrent_info, int index);

    // 是否有效
    bool Valid(BitTorrent::Torrent* torrent_handle, bool is_finished);

    // 获取种子文件列表
    //char * GetTorrentList(const BitTorrent::TorrentInfo& torrent_info);
    char * GetTorrentList(const BitTorrent::TorrentInfo& torrent_info, BitTorrent::Torrent* torrent_handle);

    // 获取文件类型
    static Utils::FileType GetFileType(const QString& file_name);

    // 获取hash，转成char*返回
    char * GetTorrentHash(const QString& hash);

    // 获取选定的文件大小
    qlonglong GetSelectedFilesSize(const BitTorrent::TorrentInfo& torrent_info, BitTorrent::Torrent* torrent_handle);

    // 设置文件名称
    void SetFileName(BitTorrent::Torrent* torrent_handle);

    // 转成utf8
    void QStringToUtf8Buf(const QString &qstr, char * buf);

    // 判断路径是否超出255字符限制
    bool IsPathLengthExceeded(const BitTorrent::TorrentInfo &torrent_info);

    // 获取分享率
    int GetTorrentShareRatio(BitTorrent::Torrent* torrent_handle);

    // 获取做种时间
    int64_t GetSeedingTime(BitTorrent::Torrent* torrent_handle);

    // 获取上传速度
    int GetUploadRate(BitTorrent::Torrent* torrent_handle);

    // 校验中
    bool IsChecking(const BitTorrent::Torrent* torrent_handle);

    // 下载中
    bool IsDownloading(const BitTorrent::Torrent* torrent_handle);

    // 设置根目录
    void SetRootFolder(const BitTorrent::TorrentInfo &torrent_info);
    void SetRootFolder(const BitTorrent::Torrent* torrent_handle);

    // 是否有根目录，创建子文件夹也算是有根目录（此时这个根目录指的是创建的子文件夹），跟原始种子列表不一样
    bool HasRoorFolder();

    // 获取根目录
    Path GetRoorFolder();

private:
    qint64 AddToFileListTree(QJsonArray& parentArray, const QStringList& pathParts, qint64 fileSize, const QString& fileType, bool is_selected, qreal progress, int index, int pathIndex);
    qint64 CalculateDirectorySize(const QJsonObject &dirObject);
    void RenameTorrentContent(BitTorrent::TorrentInfo& torrent_info, BitTorrent::AddTorrentParams& params, const QString& new_name);
//    QString GetRootFolderName(const QStringList &file_paths);
//    bool HasRootFolder(const QStringList &file_paths);

private:
    TorrentParam param_;
    QVector<BitTorrent::PeerInfo> peers_; // 需要异步获取，通过获取会卡住
    std::mutex mutex_;
    bool has_root_folder_ = false;
    Path root_folder_;
};

#endif // TORRENT_HELPER_H
