#include "torrent_helper.h"

#include <string>
#include <QObject>
#include <QString>
#include <QQueue>
#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QPointer>

#include "base/global.h"
#include "base/bittorrent/downloadpriority.h"
#include "base/bittorrent/peerinfo.h"
#include "base/bittorrent/peeraddress.h"
#include "base/unicodestrings.h"
#include "base/utils/string.h"
#include "base/utils/misc.h"
#include "base/net/geoipmanager.h"
#include "base/unicodestrings.h"

namespace Utils {
static std::map<QString, FileType> FileTypeMap = {
    {QString::fromUtf8("txt"), FileType::kTxt},

    {QString::fromUtf8("pdf"), FileType::kPdf},

    {QString::fromUtf8("docx"), FileType::kWord},    {QString::fromUtf8("doc"), FileType::kWord},
    {QString::fromUtf8("docm"), FileType::kWord},    {QString::fromUtf8("dotm"), FileType::kWord},
    {QString::fromUtf8("dotx"), FileType::kWord},

    {QString::fromUtf8("pptx"), FileType::kPPT},     {QString::fromUtf8("ppsx"), FileType::kPPT},
    {QString::fromUtf8("ppt"), FileType::kPPT},      {QString::fromUtf8("pps"), FileType::kPPT},
    {QString::fromUtf8("pptm"), FileType::kPPT},     {QString::fromUtf8("potm"), FileType::kPPT},
    {QString::fromUtf8("ppam"), FileType::kPPT},     {QString::fromUtf8("potx"), FileType::kPPT},
    {QString::fromUtf8("ppsm"), FileType::kPPT},

    {QString::fromUtf8("xlsx"), FileType::kXlxs},    {QString::fromUtf8("xlsb"), FileType::kXlxs},
    {QString::fromUtf8("xls"), FileType::kXlxs},     {QString::fromUtf8("xlsm"), FileType::kXlxs},

    {QString::fromUtf8("html"), FileType::kHtml},

    {QString::fromUtf8("7z"), FileType::kCompress},  {QString::fromUtf8("zip"), FileType::kCompress},
    {QString::fromUtf8("rar"), FileType::kCompress},

    {QString::fromUtf8("3fr"), FileType::kPicture},  {QString::fromUtf8("ai"), FileType::kPicture},
    {QString::fromUtf8("apng"), FileType::kPicture}, {QString::fromUtf8("arw"), FileType::kPicture},
    {QString::fromUtf8("avif"), FileType::kPicture}, {QString::fromUtf8("bmp"), FileType::kPicture},
    {QString::fromUtf8("cr2"), FileType::kPicture},  {QString::fromUtf8("crw"), FileType::kPicture},
    {QString::fromUtf8("dcr"), FileType::kPicture},  {QString::fromUtf8("erf"), FileType::kPicture},
    {QString::fromUtf8("gif"), FileType::kPicture},  {QString::fromUtf8("heic"), FileType::kPicture},
    {QString::fromUtf8("ico"), FileType::kPicture},  {QString::fromUtf8("jp2"), FileType::kPicture},
    {QString::fromUtf8("jpeg"), FileType::kPicture}, {QString::fromUtf8("jpe"), FileType::kPicture},
    {QString::fromUtf8("jpg"), FileType::kPicture},  {QString::fromUtf8("jpx"), FileType::kPicture},
    {QString::fromUtf8("jxr"), FileType::kPicture},  {QString::fromUtf8("k25"), FileType::kPicture},
    {QString::fromUtf8("kdc"), FileType::kPicture},  {QString::fromUtf8("mos"), FileType::kPicture},
    {QString::fromUtf8("mrw"), FileType::kPicture},  {QString::fromUtf8("nef"), FileType::kPicture},
    {QString::fromUtf8("orf"), FileType::kPicture},  {QString::fromUtf8("pef"), FileType::kPicture},
    {QString::fromUtf8("png"), FileType::kPicture},  {QString::fromUtf8("psd"), FileType::kPicture},
    {QString::fromUtf8("ptx"), FileType::kPicture},  {QString::fromUtf8("raf"), FileType::kPicture},
    {QString::fromUtf8("raw"), FileType::kPicture},  {QString::fromUtf8("rw2"), FileType::kPicture},
    {QString::fromUtf8("sr2"), FileType::kPicture},  {QString::fromUtf8("srf"), FileType::kPicture},
    {QString::fromUtf8("svg"), FileType::kPicture},  {QString::fromUtf8("tga"), FileType::kPicture},
    {QString::fromUtf8("tif"), FileType::kPicture},  {QString::fromUtf8("tiff"), FileType::kPicture},
    {QString::fromUtf8("webp"), FileType::kPicture},

    {QString::fromUtf8("mp4"), FileType::kVideo},    {QString::fromUtf8("flv"), FileType::kVideo},
    {QString::fromUtf8("webm"), FileType::kVideo},   {QString::fromUtf8("mov"), FileType::kVideo},
    {QString::fromUtf8("3gp"), FileType::kVideo},    {QString::fromUtf8("3g2"), FileType::kVideo},
    {QString::fromUtf8("rm"), FileType::kVideo},     {QString::fromUtf8("rmvb"), FileType::kVideo},
    {QString::fromUtf8("wmv"), FileType::kVideo},    {QString::fromUtf8("avi"), FileType::kVideo},
    {QString::fromUtf8("asf"), FileType::kVideo},    {QString::fromUtf8("mpg"), FileType::kVideo},
    {QString::fromUtf8("mpeg"), FileType::kVideo},   {QString::fromUtf8("ts"), FileType::kVideo},
    {QString::fromUtf8("tp"), FileType::kVideo},     {QString::fromUtf8("tpr"), FileType::kVideo},
    {QString::fromUtf8("div"), FileType::kVideo},    {QString::fromUtf8("divx"), FileType::kVideo},
    {QString::fromUtf8("dvr-ms"), FileType::kVideo}, {QString::fromUtf8("vob"), FileType::kVideo},
    {QString::fromUtf8("mkv"), FileType::kVideo},    {QString::fromUtf8("mts"), FileType::kVideo},
    {QString::fromUtf8("m2ts"), FileType::kVideo},   {QString::fromUtf8("m2t"), FileType::kVideo},
    {QString::fromUtf8("m4v"), FileType::kVideo},    {QString::fromUtf8("qt"), FileType::kVideo},
    {QString::fromUtf8("swf"), FileType::kVideo},    {QString::fromUtf8("xvvid"), FileType::kVideo},

    {QString::fromUtf8("mp3"), FileType::kMusic},    {QString::fromUtf8("m4a"), FileType::kMusic},
    {QString::fromUtf8("m4b"), FileType::kMusic},    {QString::fromUtf8("aac"), FileType::kMusic},
    {QString::fromUtf8("ogg"), FileType::kMusic},    {QString::fromUtf8("wav"), FileType::kMusic},
    {QString::fromUtf8("flac"), FileType::kMusic},   {QString::fromUtf8("ape"), FileType::kMusic},
    {QString::fromUtf8("aiff"), FileType::kMusic},   {QString::fromUtf8("aif"), FileType::kMusic},
    {QString::fromUtf8("wma"), FileType::kMusic},

    {QString::fromUtf8("apk"), FileType::kApk},      {QString::fromUtf8("exe"), FileType::kExe},
    {QString::fromUtf8("torrent"), FileType::kBt},

};
}


TorrentHelper::TorrentHelper()
{

}

TorrentParam TorrentHelper::ConvertParam(const TorrentDownloadParam& param) {
    param_.id = QString::fromUtf8(param.id);
    param_.url = QString::fromUtf8(param.url);
    param_.save_path = Path(QString::fromUtf8(param.save_path));
    param_.file_name = QString::fromUtf8(param.file_name);;
    param_.download_type = param.download_type;
    param_.item_select_list = param.item_select_list;
    param_.file_count = param.file_count;
    param_.is_download_now = param.is_download_now;
    return param_;
}

bool TorrentHelper::AddMagnet() {
    BitTorrent::AddTorrentParams params;
    params.savePath = param_.save_path;

    return BitTorrent::Session::instance()->addTorrent(param_.url, params);
}

bool TorrentHelper::AddTorrent(int file_count, BitTorrent::TorrentInfo & torrent_info) {
    BitTorrent::AddTorrentParams params;
    params.savePath = param_.save_path;
    //params.ignoreShareLimits = true;
    params.addPaused = (!param_.is_download_now);

    for (int i = 0; i < file_count; ++i) {
        for (int j = 0; j < param_.file_count; ++j) {
            int index = param_.item_select_list[j].index;
            if (index == i) {
                bool item_select = param_.item_select_list[i].is_selected;
                params.filePriorities.push_back(BitTorrent::DownloadPriority(item_select));
                break;
            }
        }
    }

   QString torrent_name = torrent_info.name();
   if (!param_.file_name.isEmpty()) {
      RenameTorrentContent(torrent_info, params, param_.file_name);
   } else {
      RenameTorrentContent(torrent_info, params, torrent_name);
   }

   return BitTorrent::Session::instance()->addTorrent(torrent_info, params);
}

bool TorrentHelper::Pause(BitTorrent::Torrent* torrent_handle) {
    if (torrent_handle) {
        torrent_handle->pause();
    }

    return true;
}

bool TorrentHelper::Resume(BitTorrent::Torrent* torrent_handle, bool is_failed){
    if (torrent_handle) {
        if (is_failed) {
            torrent_handle->forceRecheck();
        }
        torrent_handle->resume();
    }

    return true;
}


char * TorrentHelper::GetSeedingInfo(BitTorrent::Torrent* torrent_handle) {
    const qlonglong dlDuration = torrent_handle->activeTime() - torrent_handle->finishedTime();
    QString dlAvg;
    if (dlDuration <= 0) {
        dlAvg = QString::number(torrent_handle->downloadPayloadRate());
    } else {
        dlAvg = QString::number(torrent_handle->totalDownload() / dlDuration);
    }
    const qlonglong ulDuration = torrent_handle->activeTime();
    QString ulAvg;
    if (ulDuration <= 0) {
       ulAvg = QString::number(torrent_handle->uploadPayloadRate());
    } else {
       ulAvg = QString::number(torrent_handle->totalUpload() / ulDuration);
    }

    QJsonObject json_object;
    json_object[kTaskIdKey] = param_.id; // 任务id;
    json_object[kTaskNameKey] = torrent_handle->name(); // 任务名
    json_object[kPrivateKey] = torrent_handle->isPrivate(); // 是否是私有种子
    json_object[kShareRatioKey] = (torrent_handle->realRatio() > BitTorrent::Torrent::MAX_RATIO) ? C_INFINITY : Utils::String::fromDouble(torrent_handle->realRatio(), 2);
    json_object[kDownloadedSizeKey] = QString::number(torrent_handle->totalDownload()); // 已下载大小，单位：字节
    json_object[kUploadedSizeKey] = QString::number(torrent_handle->totalUpload());// 已上传大小，单位：字节
    json_object[kActiveSeedsCountKey] = torrent_handle->seedsCount(); // 活跃种子数;
    json_object[kTotalSeedsCountKey] = torrent_handle->totalSeedsCount();// 种子总数
    json_object[kActivePeerCountKey] = torrent_handle->peersCount(); // 活跃用户数
    json_object[kTotalPeerCountKey] = torrent_handle->totalPeersCount();// 用户总数
    json_object[kSeedingTimeKey] = torrent_handle->finishedTime(); // 已做种时间
    json_object[kConnectionsCountKey] = torrent_handle->connectionsCount(); // 连接数
    json_object[kMaxConnectionsCountKey] = torrent_handle->connectionsLimit();// 最大连接数
    json_object[kDownloadSpeedKey] = QString::number(torrent_handle->downloadPayloadRate()); // 下载速度，单位：字节/s
    json_object[kUploadSpeedKey] = QString::number(torrent_handle->uploadPayloadRate());// 上传速度/s
    json_object[kDownloadAverageSpeedKey] = dlAvg; // 下载平均速度，单位：字节/s
    json_object[kUploadAverageSpeedKey] = ulAvg; // 上传平均速度，单位：字节/s
    json_object[kCreatorKey] =  torrent_handle->creator(); // 创建者
    json_object[kCreateTimeKey] = QLocale().toString(torrent_handle->creationDate(), QLocale::ShortFormat); // 创建时间
    json_object[kTotalSizeKey] = QString::number(torrent_handle->wantedSize()); // 选择文件的总大小
    json_object[kTorrentSizeKey] = QString::number(torrent_handle->totalSize());// 整个种子包含的文件总大小
    BitTorrent::InfoHash hash = torrent_handle->infoHash();
    QString hash_v1 = hash.v1().toString();
    QString hash_v2 = hash.v2().toString();
    json_object[kHashV1Key] = hash_v1.isEmpty() ? kNA:hash_v1;
    json_object[kHashV2Key] = hash_v2.isEmpty() ? kNA:hash_v2;
    json_object[kSavePathKey] = param_.save_path.toString();
    json_object[kCommentKey] = torrent_handle->comment(); // 注释

    // tracker tab
    QJsonArray tracker_array = GetTrackerList(torrent_handle);

    // peers tab
    QJsonArray peers_array = GetPeersList(torrent_handle);

    // 创建一个 QJsonObject
    json_object[kTrackers] = tracker_array;
    json_object[kPeers] = peers_array;

    QJsonDocument json_document(json_object);
    QByteArray utf8_data = json_document.toJson(QJsonDocument::Compact);
    int data_size = utf8_data.size();
    char * buf = new char[data_size + 1];
    memcpy(buf, utf8_data.constData(), data_size);
    // 添加 null 结尾字符
    buf[data_size] = '\0';
    return buf;
}

QJsonArray TorrentHelper::GetTrackerList(BitTorrent::Torrent* torrent_handle) {
    QJsonArray json_array;
    LoadStickyItems(&json_array, torrent_handle);
    LoadActualItems(&json_array, torrent_handle);
    return json_array;
}

void TorrentHelper::LoadStickyItems(QJsonArray* json_array, BitTorrent::Torrent* torrent_handle) {
    // 创建 JSON 对象
    QJsonObject dht_json_object;
    QJsonObject pex_json_object;
    QJsonObject lsd_json_object;

    dht_json_object[kTierKey] = kEmpty;
    pex_json_object[kTierKey] = kEmpty;
    lsd_json_object[kTierKey] = kEmpty;
    dht_json_object[kUrlKey] = kDHT;
    pex_json_object[kUrlKey] = kPeX;
    lsd_json_object[kUrlKey] = kLSD;

    // load DHT information
    if (BitTorrent::Session::instance()->isDHTEnabled() && !torrent_handle->isPrivate())
        dht_json_object[kStatusKey] = kWorking;
    else
        dht_json_object[kStatusKey] = kDisabled;

    // Load PeX Information
    if (BitTorrent::Session::instance()->isPeXEnabled() && !torrent_handle->isPrivate())
        pex_json_object[kStatusKey] = kWorking;
    else
        pex_json_object[kStatusKey] = kDisabled;

    // Load LSD Information
    if (BitTorrent::Session::instance()->isLSDEnabled() && !torrent_handle->isPrivate())
        lsd_json_object[kStatusKey] = kWorking;
    else
        lsd_json_object[kStatusKey] = kDisabled;

    // Count peers from DHT, PeX, LSD
    uint seedsDHT = 0, seedsPeX = 0, seedsLSD = 0, peersDHT = 0, peersPeX = 0, peersLSD = 0;
    using TorrentPtr = QPointer<const BitTorrent::Torrent>;
    torrent_handle->fetchPeerInfo([this, torrent = TorrentPtr(torrent_handle)](const QVector<BitTorrent::PeerInfo> &peers)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        peers_ = std::move(peers);
    });

    QVector<BitTorrent::PeerInfo> peers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        peers  = peers_;
    }

    for (const BitTorrent::PeerInfo &peer : asConst(peers)) {
        if (peer.isConnecting()) continue;

        if (peer.fromDHT()) {
            if (peer.isSeed())
                ++seedsDHT;
            else
                ++peersDHT;
        }
        if (peer.fromPeX()) {
            if (peer.isSeed())
                ++seedsPeX;
            else
                ++peersPeX;
        }
        if (peer.fromLSD()) {
            if (peer.isSeed())
                ++seedsLSD;
            else
                ++peersLSD;
        }
    }

    dht_json_object[kPeersKey] = QString::number(peersDHT);
    dht_json_object[kSeedsKey] = QString::number(seedsDHT);
    pex_json_object[kPeersKey] = QString::number(peersPeX);
    pex_json_object[kSeedsKey] = QString::number(seedsPeX);
    lsd_json_object[kPeersKey] = QString::number(peersLSD);
    lsd_json_object[kSeedsKey] = QString::number(seedsLSD);

    if (torrent_handle->isPrivate()) {
        dht_json_object[kMsgKey] = kPrivate;
        pex_json_object[kMsgKey] = kPrivate;
        lsd_json_object[kMsgKey] = kPrivate;
    }

    json_array->append(dht_json_object);
    json_array->append(pex_json_object);
    json_array->append(lsd_json_object);
}

void TorrentHelper::LoadActualItems(QJsonArray* json_array, BitTorrent::Torrent* torrent_handle) {
    // Load actual trackers information

    for (const BitTorrent::TrackerEntry &entry : asConst(torrent_handle->trackers())) {
        const QString trackerURL = entry.url;

        QJsonObject json_object;
        json_object[kTierKey] = entry.tier;
        json_object[kUrlKey] = trackerURL;

        switch (entry.status) {
        case BitTorrent::TrackerEntry::Working:
            json_object[kStatusKey] = kWorking;
            break;
        case BitTorrent::TrackerEntry::Updating:
            json_object[kStatusKey] = kUpdating;
            break;
        case BitTorrent::TrackerEntry::NotWorking:
            json_object[kStatusKey] = kNotWorking;
            break;
        case BitTorrent::TrackerEntry::NotContacted:
            json_object[kStatusKey] = kNotContacted;
            break;
        }
        json_object[kMsgKey] = entry.message;
        json_object[kPeersKey] = (entry.numPeers > -1)
                ? QString::number(entry.numPeers)
                : kNA;
        json_object[kSeedsKey] = (entry.numSeeds > -1)
                ? QString::number(entry.numSeeds)
                : kNA;
        json_array->append(json_object);
    }
}

QJsonArray TorrentHelper::GetPeersList(BitTorrent::Torrent* torrent_handle){
    QJsonArray peers_array;
    QVector<BitTorrent::PeerInfo> peers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        peers  = peers_;
    }

    //const QVector<BitTorrent::PeerInfo> peers = torrent_handle->peers();
    for (const BitTorrent::PeerInfo &peer : peers) {
        if (peer.address().ip.isNull()) {
            return peers_array;
        }
        QJsonObject json_object;
        json_object[kCountryKey] = peer.country();
        json_object[kIpKey] = peer.address().ip.toString();
        json_object[kPortKey] = QString::number(peer.address().port);
        json_object[kConnectionTypeKey] = peer.connectionType();
        json_object[kDownloadProgressKey] = peer.progress();
        json_object[kDownloadSpeedKey] =  peer.payloadDownSpeed();
        json_object[kUploadSpeedKey] =  peer.payloadUpSpeed();
        json_object[kTotalDownloadSizeKey] =  peer.totalDownload();
        json_object[kTotalUploadSizeKey] =  peer.totalUpload();
        peers_array.append(json_object);
    }
    return peers_array;
}

void TorrentHelper::ReleaseBuf(const char * buf) {
    if (buf) {
        delete [] buf;
        buf = NULL;
    }
}


void TorrentHelper::GetProgressInfo(BitTorrent::Torrent* torrent_handle, TorrentDownloadProgressInfo** info) { 
//    if (has_root_folder_) {
//        QStringToUtf8Buf(root_folder_.toString(), (*info)->file_name);
//    } else {
//        QStringToUtf8Buf(torrent_handle->name(), (*info)->file_name);
//    }

    BitTorrent::TorrentState state = torrent_handle->state();

    (*info)->total_progress = torrent_handle->progress()*100;
    (*info)->download_speed =  torrent_handle->downloadPayloadRate();  //B/s
    (*info)->upload_speed =  torrent_handle->uploadPayloadRate();  //B/s
    (*info)->state = static_cast<int>(state);
    (*info)->is_seed = torrent_handle->isFinished();

    if (torrent_handle->hasFileIoError()) {
        (*info)->err_code = DownloadErrorCode::kFileIoError;
    }
}

int TorrentHelper::GetSelectedTorrentFileCount(BitTorrent::Torrent* torrent_handle) {
    int selected_count = 0;

    QVector<BitTorrent::DownloadPriority> file_priorities = torrent_handle->filePriorities();
    int size = file_priorities.size();
    // file_priorities的size最开始程序恢复的时候值不对，做个异常判断
    if (size > 1000) {
        return selected_count;
    }
    for (int i = 0; i < size; ++i) {
        if (int(file_priorities[i]) <= 0) {
            continue;
        }
        selected_count++;
    }
    return selected_count;
}

FileBaseInfo TorrentHelper::GetFileBaseInfo(const BitTorrent::TorrentInfo& torrent_info, int index) {
    FileBaseInfo file_base_info;
    QString name = torrent_info.filePath(index).filename();
    QStringToUtf8Buf(name, file_base_info.file_name);

    QFileInfo file(name);
    QString suffix = file.suffix();
    QStringToUtf8Buf(suffix, file_base_info.file_type);
    return file_base_info;
}

bool TorrentHelper::Valid(BitTorrent::Torrent* torrent_handle, bool is_finished) {
    if (torrent_handle && (torrent_handle->isFinished() || torrent_handle->isCompleted() || is_finished)) {
//        if (torrent_handle->hasMissingFiles()) {
//            return false;
//        }

//        QByteArray utf8_data = torrent_handle->error().toUtf8();
//        std::string err_msg(utf8_data.data(), utf8_data.size());
//        BitTorrent::TorrentState state = torrent_handle->state();
//        if (err_msg.find("file not found") != std::string::npos ||
//            err_msg.find("no such file") != std::string::npos || state == BitTorrent::TorrentState::MissingFiles) {
//            return false;
//        }

        // 目前根据根目录是否存在来判断是否有效，如果没有根目录，就判断文件是否存在
        BitTorrent::TorrentInfo torrent_info = torrent_handle->info();
        if (torrent_info.isValid()) {
            if (has_root_folder_) { 
               Path root = param_.save_path / root_folder_;
               if (!root.exists()) {
                   return false;
               }
            } else {
                QVector<BitTorrent::DownloadPriority> file_priorities = torrent_handle->filePriorities();
                for (int i = 0; i < file_priorities.size(); ++i) {
                    // 跳过未选择的文件
                    if (int(file_priorities[i]) <= 0) {
                        continue;
                    }
                    Path file_path = param_.save_path / torrent_handle->actualFilePath(i);
                    if (!file_path.exists()) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

Utils::FileType TorrentHelper::GetFileType(const QString& file_name) {
  Utils::FileType file_type = Utils::FileType(0);
  do {
    int pos = file_name.lastIndexOf(QString::fromUtf8("."));
    if (pos == -1) {
      break;
    }

    QString suffix = file_name.mid(pos + 1);
    // 转换小写
    QString  lower_suffix = suffix.toLower();
    auto iter = Utils::FileTypeMap.find(lower_suffix);
    if (iter != Utils::FileTypeMap.end()) {
      file_type = iter->second;
    }
  } while (0);

  return file_type;
}

char* TorrentHelper::GetTorrentList(const BitTorrent::TorrentInfo& torrent_info, BitTorrent::Torrent* torrent_handle) {
    QJsonObject root_object;

    PathList filePaths = torrent_info.filePaths();
    QString file_name = param_.file_name;
    if (!file_name.isEmpty()) {
        root_object[kTaskNameKey] = file_name;
    } else if (has_root_folder_) {
        root_object[kTaskNameKey] = root_folder_.toString();
    } else {
        root_object[kTaskNameKey] = torrent_info.name();
    }

    root_object[kTaskSizeKey] = QString::number(torrent_info.totalSize());
    root_object[kTaskProgressKey] = torrent_handle ? torrent_handle->progress() : 0.0;

    QJsonArray childrenArray;
    int files_count = torrent_info.filesCount();

    QVector<BitTorrent::DownloadPriority> file_priorities;
    QVector<qreal> files_progress;
    if (torrent_handle) {
        file_priorities = torrent_handle->filePriorities();
        files_progress = torrent_handle->filesProgress();
    }

    int priority_size = file_priorities.size();
    int files_progress_size = files_progress.size();
    QHash<QString, int> nameIndexMap;
    for (int i = 0; i < files_count; ++i) {
        Path filePath = torrent_info.filePath(i);
        qint64 fileSize = torrent_info.fileSize(i);
        QString fileName = filePath.filename();
        QString fileType = QString::number(GetFileType(fileName));
        bool is_selected = true;
        qreal progress = 0;
        if (priority_size > 0 && i < priority_size) {
            is_selected = int(file_priorities[i]) > 0;
        }
        if (files_progress_size > 0 && i < files_progress_size) {
            progress = torrent_handle ? files_progress[i] * 100.0 : 0.0;
        }

        QStringList pathParts = filePath.toString().split(kSeparatorKey, Qt::SkipEmptyParts);
        AddToFileListTree(childrenArray, pathParts, fileSize, fileType, is_selected, progress, i, 0);
    }

    // 如果只有一个顶层元素且包含 "children" 属性，直接将其子元素添加到 root_object 的 "children" 数组中
      if (childrenArray.size() == 1) {
          QJsonObject singleObject = childrenArray[0].toObject();
          if (singleObject.contains(kChildrenKey) && singleObject[kChildrenKey].toArray().size() > 0) {
              QJsonArray topLevelChildren = singleObject[kChildrenKey].toArray();
              root_object[kChildrenKey] = topLevelChildren;
          } else {
              root_object[kChildrenKey] = childrenArray;
          }
      } else {
          root_object[kChildrenKey] = childrenArray;
      }


    QJsonDocument json_document(root_object);
    QByteArray utf8_data = json_document.toJson(QJsonDocument::Compact);
    int data_size = utf8_data.size();
    char * buf = new char[data_size + 1];
    memcpy(buf, utf8_data.constData(), data_size);
    buf[data_size] = '\0';
    return buf;
}

qint64 TorrentHelper::AddToFileListTree(QJsonArray& parentArray, const QStringList& pathParts, qint64 fileSize, const QString& fileType, bool is_selected, qreal progress, int index, int pathIndex) {
    if (pathIndex >= pathParts.size()) {
        return 0;
    }

    QString currentName = pathParts[pathIndex];
    bool found = false;
    int i = 0;

    for (; i < parentArray.size(); ++i) {
        QJsonObject childObject = parentArray[i].toObject();
        if (childObject[kChildNameKey].toString() == currentName) {
            found = true;
            break;
        }
    }

    qint64 currentSize = 0;

    if (!found) {
        QJsonObject newObject;
        newObject[kChildNameKey] = currentName;
        if (pathIndex == pathParts.size() - 1) {
            newObject[kChildIndexKey] = index;
            newObject[kChildSizeKey] = QString::number(fileSize);
            newObject[kChildTypeKey] = fileType;
            newObject[kChildSelectedKey] = is_selected;
            newObject[kChildProgressKey] = progress;
            currentSize = fileSize;
        } else {
            newObject[kChildrenKey] = QJsonArray();
            newObject[kChildSizeKey] = QString::number(0);
            newObject[kChildIndexKey] = -1;
        }
        parentArray.append(newObject);
    }

    if (pathIndex < pathParts.size() - 1) {
        QJsonObject childObject = parentArray[i].toObject();
        QJsonArray childArray = childObject[kChildrenKey].toArray();
        currentSize = AddToFileListTree(childArray, pathParts, fileSize, fileType, is_selected, progress, index, pathIndex + 1);
        childObject[kChildrenKey] = childArray;

        if (currentSize > 0) {
            qint64 currentDirSize = childObject[kChildSizeKey].toString().toLongLong();
            childObject[kChildSizeKey] = QString::number(currentDirSize + currentSize);
            parentArray.replace(i, childObject);
        }
    }

    return currentSize;
}

char * TorrentHelper::GetTorrentHash(const QString& hash) {
    QByteArray utf8_data = hash.toUtf8();
    int data_size = utf8_data.size();
    char * buf = new char[data_size + 1];
    memcpy(buf, utf8_data.constData(), data_size);
    buf[data_size] = '\0';
    return buf;
}

qlonglong TorrentHelper::GetSelectedFilesSize(const BitTorrent::TorrentInfo& torrent_info,  BitTorrent::Torrent* torrent_handle) {
    unsigned long long size = 0;
    if (torrent_handle) {
         size = torrent_handle->wantedSize();
    }

    return size;
}

void TorrentHelper::SetFileName(BitTorrent::Torrent* torrent_handle) {
    if (torrent_handle) {
        if (!param_.file_name.isEmpty()) {
             torrent_handle->setName(param_.file_name);
        }
    }
}

void TorrentHelper::RenameTorrentContent(BitTorrent::TorrentInfo& torrent_info, BitTorrent::AddTorrentParams& params, const QString& new_name)
{
    if (!torrent_info.isValid()) return;

    PathList filePaths = torrent_info.filePaths();
    PathList renamePaths = filePaths;
    const Path oldRootFolder = Path::findRootFolder(filePaths);
    has_root_folder_ = !oldRootFolder.isEmpty();
    bool createSubfolder = BitTorrent::Session::instance()->torrentContentLayout() == BitTorrent::TorrentContentLayout::Subfolder ? true: false;

    // 如果存在根文件夹，则重命名根文件夹
   if (has_root_folder_) {
        for (int i = 0; i < filePaths.size(); ++i) {
            Path oldFilePath = filePaths[i];
            Path newFilePath = Path(new_name) / oldRootFolder.relativePathOf(oldFilePath);
            renamePaths[i] = newFilePath;
            qDebug() << "oldFilePath: " << oldFilePath.toString() << ", newFilePath: " <<  newFilePath.toString();
            //torrent_info.renameFile(i, newFilePath);
        }
    }
    // 如果不存在根文件夹且允许创建子文件夹，则创建一个新的根文件夹
    else if (filePaths.size() >= 1 && createSubfolder) {
       for (int i = 0; i < filePaths.size(); ++i) {
           Path oldFilePath = filePaths[i];
           Path newFilePath = Path(new_name) / oldFilePath;
           renamePaths[i] = newFilePath;
           qDebug() << "oldFilePath: " << oldFilePath.toString() << ", newFilePath: " <<  newFilePath.toString();
           //torrent_info.renameFile(i, newFilePath);
       }
   }
   params.filePaths = renamePaths;
   root_folder_ = Path::findRootFolder(renamePaths);
   has_root_folder_ = !root_folder_.isEmpty();
}

void TorrentHelper::QStringToUtf8Buf(const QString &qstr, char * buf) {
    QByteArray utf8_data = qstr.toUtf8();
    int data_size = utf8_data.size();
    memcpy(buf, utf8_data.constData(), data_size);
    // 添加空字符终止符
    buf[data_size] = '\0';
}

bool TorrentHelper::IsPathLengthExceeded(const BitTorrent::TorrentInfo &torrent_info)
{
    PathList filePaths = torrent_info.filePaths();
    const Path oldRootFolder = Path::findRootFolder(filePaths);
    bool hasRootFolder = !oldRootFolder.isEmpty();

    Path save_path = param_.save_path;
    Path file_name = Path(param_.file_name);
    for (int i = 0; i < filePaths.size(); ++i) {
        Path oldFilePath = filePaths[i];
        Path newFilePath;
        if (hasRootFolder) {
            newFilePath = file_name / oldRootFolder.relativePathOf(oldFilePath);
        } else {
           newFilePath = file_name / oldFilePath;
        }

        Path oldAbsolutePath =  save_path / oldFilePath;
        Path newAbsolutePath = save_path / newFilePath;
        if (oldAbsolutePath.toString().length() > 260 || newAbsolutePath.toString().length() > 260) {
            return true;
        }
    }

    return false;
}

int TorrentHelper::GetTorrentShareRatio(BitTorrent::Torrent* torrent_handle) {
    int share_ratio = 0;
    if (torrent_handle) {
        share_ratio = torrent_handle->realRatio() * 100;
    }
    return share_ratio;
}

int64_t TorrentHelper::GetSeedingTime(BitTorrent::Torrent* torrent_handle) {
    int64_t seeding_time = 0;
    if (torrent_handle) {
        seeding_time = torrent_handle->finishedTime();
    }
    return seeding_time;
}

int TorrentHelper::GetUploadRate(BitTorrent::Torrent* torrent_handle) {
    int upload_rate = 0;
    if (torrent_handle) {
        upload_rate = torrent_handle->uploadPayloadRate();
    }
    return upload_rate;
}

bool TorrentHelper::IsChecking(const BitTorrent::Torrent* torrent_handle) {
    if (torrent_handle) {
        return torrent_handle->isChecking();
    }
    return false;
}

bool TorrentHelper::IsDownloading(const BitTorrent::Torrent* torrent_handle) {
    if (torrent_handle) {
        return torrent_handle->isDownloading();
    }
    return false;
}

void TorrentHelper::SetRootFolder(const BitTorrent::TorrentInfo &torrent_info) {
    if (torrent_info.isValid()) {
        root_folder_ = Path::findRootFolder(torrent_info.filePaths());
        has_root_folder_ = !root_folder_.isEmpty();
        if (!has_root_folder_) {
            bool create_subfolder = BitTorrent::Session::instance()->torrentContentLayout() == BitTorrent::TorrentContentLayout::Subfolder ? true: false;
            if (create_subfolder) {
                QFileInfo file_info(torrent_info.name());
                QString file_name_without_suffix = file_info.completeBaseName();
                root_folder_ = Path(file_name_without_suffix);
                has_root_folder_ = !root_folder_.isEmpty();
            }
        }
        qDebug() << "root_folder: " << root_folder_.toString();
    }
}

void TorrentHelper::SetRootFolder(const BitTorrent::Torrent* torrent_handle) {
    if (torrent_handle) {
        root_folder_ = Path::findRootFolder(torrent_handle->filePaths());
        has_root_folder_ = !root_folder_.isEmpty();
        if (!has_root_folder_) {
            bool create_subfolder = BitTorrent::Session::instance()->torrentContentLayout() == BitTorrent::TorrentContentLayout::Subfolder ? true: false;
            if (create_subfolder) {
                QFileInfo file_info(torrent_handle->name());
                QString file_name_without_suffix = file_info.completeBaseName();
                root_folder_ = Path(file_name_without_suffix);
                has_root_folder_ = !root_folder_.isEmpty();
            }
        }
        qDebug() << "root_folder: " << root_folder_.toString();
    }
}

bool TorrentHelper::HasRoorFolder() {
    return has_root_folder_;
}

Path TorrentHelper::GetRoorFolder() {
    return root_folder_;
}


