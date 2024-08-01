#include <string>
#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "base/global.h"
#include "base/bittorrent/downloadpriority.h"
#include "base/bittorrent/peerinfo.h"
#include "base/bittorrent/peeraddress.h"
#include "base/unicodestrings.h"
#include "base/utils/string.h"
#include "base/utils/misc.h"
#include "base/net/geoipmanager.h"
#include "base/unicodestrings.h"
#include "torrent_file_download_task_impl.h"

TorrentlFileDownloadTaskImpl::TorrentlFileDownloadTaskImpl() {
    event_obj_ = new SubscribeEvent();
    event_obj_->moveToThread(BitTorrent::Session::instance()->thread());
}

void TorrentlFileDownloadTaskImpl::SetParam(const TorrentDownloadParam& param) {
    param_ = torrent_helper_.ConvertParam(param);
    const nonstd::expected<BitTorrent::TorrentInfo, QString> result = BitTorrent::TorrentInfo::loadFromFile(Path(param_.url));
    if (result) {
        torrent_info_ = result.value();
        torrent_handle_ = GetTorrentHandle();
        torrent_helper_.SetRootFolder(torrent_handle_);
    }
}

DownloadErrorCode TorrentlFileDownloadTaskImpl::Start()
{
    DownloadErrorCode ret_err = DownloadErrorCode::kNormal;
    BitTorrent::AddTorrentParams torrent_params;
    do {
        qDebug() << "start download! id = " << param_.id << ", "
                 << "url = " << param_.url << ", "
                 << "save_path = " << param_.save_path.toString() << ", "
                 << "type = torrent_file, "
                 << "file_count = " << param_.file_count;

        ret_err = CheckCondition(param_.url.toStdString().c_str());
        if (ret_err != DownloadErrorCode::kNormal){
            break;
        }

        int file_count = torrent_info_.filesCount();
        bool ret = torrent_helper_.AddTorrent(file_count, torrent_info_);
        ret_err = ret?DownloadErrorCode::kNormal:DownloadErrorCode::kAddTorrentTaskFailed;
        std::string v1 = torrent_info_.infoHash().v1().toString().toStdString();
        std::string v2 = torrent_info_.infoHash().v2().toString().toStdString();
        qDebug() << "v1: " << v1.c_str() << ", v2: " << v2.c_str();

    } while(0);

    qDebug() << "add torrent file is " << ret_err;
    return ret_err;
}

bool TorrentlFileDownloadTaskImpl::Pause() {
    return torrent_helper_.Pause(torrent_handle_);
}

bool TorrentlFileDownloadTaskImpl::Resume(bool is_failed){
    return torrent_helper_.Resume(torrent_handle_, is_failed);
}

bool TorrentlFileDownloadTaskImpl::Cancel(bool is_delete_local_file) {
    DeleteOption deleteOption = DeleteOption::DeleteTorrent;
    if (is_delete_local_file) {
        deleteOption = DeleteOption::DeleteTorrentAndFiles;
    }

    if (!torrent_info_.isValid() || !BitTorrent::Session::instance()->isKnownTorrent(torrent_info_.infoHash())) {
        return true;
    }

    BitTorrent::Session::instance()->deleteTorrent(torrent_info_.infoHash().toTorrentID(), deleteOption);
    return true;
}

bool TorrentlFileDownloadTaskImpl::GetProgressInfo(DownloadProgressInfo* info) {
    TorrentDownloadProgressInfo * torrent_progress_info = static_cast<TorrentDownloadProgressInfo *>(info);
    if (!torrent_progress_info) {
        return false;
    }

    if (!torrent_info_.isValid()) {
        qDebug() << "torrent_info_ is invalid, id is " << param_.id;
        return false;
    }

    std::string name = torrent_info_.name().toStdString();
    torrent_helper_.QStringToUtf8Buf(torrent_info_.name(), torrent_progress_info->file_name);

    if (!torrent_handle_) {
        qDebug() << "torrent handle is null, id is " << param_.id;
        return false;
    }

    torrent_helper_.GetProgressInfo(torrent_handle_, &torrent_progress_info);
    return true;
}

int TorrentlFileDownloadTaskImpl::GetTorrentFileCount() {
    if (!torrent_info_.isValid()) {
        qDebug() << "ParseSource:: torrent_info_ is invalid, id is " << param_.id;
        return 0;
    }
    return torrent_info_.filesCount();
}

FileBaseInfo TorrentlFileDownloadTaskImpl::GetTorrentFileInfoByIndex(int index) {
    FileBaseInfo file_base_info;
    if (!torrent_info_.isValid()) {
        qDebug() << "ParseSource:: torrent_info_ is invalid, id is " << param_.id;
        return file_base_info;
    }

    file_base_info = torrent_helper_.GetFileBaseInfo(torrent_info_, index);
    return file_base_info;
}

FileBaseInfo TorrentlFileDownloadTaskImpl::GetTorrentFileBaseInfo() {
    FileBaseInfo base_info;
    if (!torrent_info_.isValid()) {
        qDebug() << "ParseSource:: torrent_info_ is invalid, id is " << param_.id;
        return base_info;
    }

    std::string file_name = param_.file_name.toStdString();
    if (!file_name.empty()) {
        memcpy(base_info.file_name, file_name.c_str(), file_name.size());
    } else {
      torrent_helper_.QStringToUtf8Buf(torrent_info_.name(), base_info.file_name);
    }

    base_info.file_size = torrent_helper_.GetSelectedFilesSize(torrent_info_, torrent_handle_);

    QFileInfo file(torrent_info_.name());
    QString suffix = file.suffix();
    torrent_helper_.QStringToUtf8Buf(suffix, base_info.file_type);

    return base_info;
}

DownloadErrorCode TorrentlFileDownloadTaskImpl::CheckCondition(const char * source) {
    DownloadErrorCode err = DownloadErrorCode::kNormal;
    do {
        const nonstd::expected<BitTorrent::TorrentInfo, QString> result = BitTorrent::TorrentInfo::loadFromFile(Path(param_.url));
        if (!result) {
            qDebug() << "CheckCondition: " << source << " is not valid torrent file";
            err = DownloadErrorCode::kTorrentFileInvalid;
            break;
        }

        torrent_info_ = result.value();
        if (!torrent_info_.isValid()) {
            qDebug() << "CheckCondition: " << source << " torrent_info is valid";
            err = DownloadErrorCode::kTorrentFileInvalid;
            break;
        }

        if (BitTorrent::Session::instance()->isKnownTorrent(torrent_info_.infoHash())) {
            BitTorrent::Torrent *const torrent = BitTorrent::Session::instance()->findTorrent(torrent_info_.infoHash());
            if (torrent){
                torrent->addTrackers(torrent_info_.trackers());
                torrent->addUrlSeeds(torrent_info_.urlSeeds());
            }
        }

        if (torrent_helper_.IsPathLengthExceeded(torrent_info_)) {
            err = DownloadErrorCode::kPathExceedLimit;
            break;
        }

    } while(0);

    return err;
}

bool TorrentlFileDownloadTaskImpl::IsValid(const char * source) {
    if (!torrent_info_.isValid()) {
        return false;
    }
    return true;
}

int TorrentlFileDownloadTaskImpl::CreateMagnetUri(char * buf) {
    int size = 0;
    if (!torrent_handle_) {
       return size;
    }

    std::string magnet_uri = torrent_handle_->createMagnetURI().toStdString();
    size = magnet_uri.size();
    if (!buf) {
        return size;
    }

    torrent_helper_.QStringToUtf8Buf(torrent_handle_->createMagnetURI(), buf);

    return 0;
}


char * TorrentlFileDownloadTaskImpl::GetSeedingInfo() {
    char * buf = NULL;
    if (!torrent_handle_) {
        return buf;
    }
    return torrent_helper_.GetSeedingInfo(torrent_handle_);
}

void TorrentlFileDownloadTaskImpl::ReleaseBuf(const char * buf) {
    torrent_helper_.ReleaseBuf(buf);
}


bool TorrentlFileDownloadTaskImpl::Valid(bool is_finished) {
    return torrent_helper_.Valid(torrent_handle_, is_finished);
}

char * TorrentlFileDownloadTaskImpl::GetTorrentList() {
    if (!torrent_info_.isValid()) {
        return NULL;
    }

    return torrent_helper_.GetTorrentList(torrent_info_, torrent_handle_);
}

const char * TorrentlFileDownloadTaskImpl::GetTaskHash() {
    if (!torrent_info_.isValid()) {
        return NULL;
    }

    QString hash = torrent_info_.infoHash().toTorrentID().toString();
    return torrent_helper_.GetTorrentHash(hash);
}

int TorrentlFileDownloadTaskImpl::GetTorrentShareRatio() {
    return torrent_helper_.GetTorrentShareRatio(torrent_handle_);
}

int64_t TorrentlFileDownloadTaskImpl::GetSeedingTime() {
    return torrent_helper_.GetSeedingTime(torrent_handle_);
}

int TorrentlFileDownloadTaskImpl::GetUploadRate() {
    return torrent_helper_.GetUploadRate(torrent_handle_);
}

BitTorrent::Torrent * TorrentlFileDownloadTaskImpl::GetTorrentHandle() {
    BitTorrent::Torrent * torrent = nullptr;

    do {
        if (!torrent_info_.isValid()) {
            qDebug() << "GetTorrentHandle: " << " is not valid url";
            break;
        }

        if (BitTorrent::Session::instance()->isKnownTorrent(torrent_info_.infoHash())) {
            torrent = BitTorrent::Session::instance()->findTorrent(torrent_info_.infoHash());
        }

    } while(0);

    return torrent;
}

void TorrentlFileDownloadTaskImpl::SetTorrentAddedCallback(void* context, PfnTorrentAdded callback) {
   context_ = context;
   callback_ = callback;
   event_obj_->SetTorrentHash(torrent_info_.infoHash());
   event_obj_->SetTorrentAddedCallback(std::bind(&TorrentlFileDownloadTaskImpl::OnTorrentAdded, this, std::placeholders::_1));
}

bool TorrentlFileDownloadTaskImpl::IsChecking() {
   return torrent_helper_.IsChecking(torrent_handle_);
}

bool TorrentlFileDownloadTaskImpl::IsDownloading() {
    return torrent_helper_.IsDownloading(torrent_handle_);
}

void TorrentlFileDownloadTaskImpl::OnTorrentAdded(BitTorrent::Torrent *const torrent) {
    if (!torrent_handle_) {
        torrent_handle_ = torrent;
    }
     torrent_helper_.SetFileName(torrent_handle_);
    if (callback_) {
        callback_(context_);
    }
}




