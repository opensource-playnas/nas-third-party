#include <string>
#include <QDebug>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include "magnet_download_task_impl.h"

MagnetDownloadTaskImpl::MagnetDownloadTaskImpl() {
    meta_data_ = new SubscribeEvent();
    meta_data_->moveToThread(BitTorrent::Session::instance()->thread());
}

void MagnetDownloadTaskImpl::SetParam(const TorrentDownloadParam& param) {
    param_ = torrent_helper_.ConvertParam(param);
    torrent_handle_ = GetTorrentHandle();
    if (torrent_handle_) {
       torrent_info_ = torrent_handle_->info();
       torrent_helper_.SetRootFolder(torrent_info_);
    }
}

DownloadErrorCode MagnetDownloadTaskImpl::Start()
{
    DownloadErrorCode ret_err = DownloadErrorCode::kNormal;
    BitTorrent::AddTorrentParams torrent_params;
    do {
        qDebug() << "start download! id = " << param_.id << ", "
                 << "url = " << param_.url << ", "
                 << "save_path = " << param_.save_path.toString() << ", "
                 << "type = magnet, "
                 << "file_count = " << param_.file_count;

        bool ret = torrent_helper_.AddMagnet();
        ret_err = ret?DownloadErrorCode::kNormal:DownloadErrorCode::kAddMagnetTaskFailed;
    } while(0);

    qDebug() << "add torrent ret is " << ret_err;
    return ret_err;
}

bool MagnetDownloadTaskImpl::Pause() {  
    return torrent_helper_.Pause(torrent_handle_);
}

bool MagnetDownloadTaskImpl::Resume(bool is_failed){
    return torrent_helper_.Resume(torrent_handle_, is_failed);
}

bool MagnetDownloadTaskImpl::Cancel(bool is_delete_local_file) {
    const BitTorrent::MagnetUri magnetUri(param_.url);
    DeleteOption deleteOption = DeleteOption::DeleteTorrent;
    if (is_delete_local_file) {
        deleteOption = DeleteOption::DeleteTorrentAndFiles;
    }

    BitTorrent::Session::instance()->deleteTorrent(magnetUri.infoHash().toTorrentID(), deleteOption);
    BitTorrent::Session::instance()->cancelDownloadMetadata(magnetUri.infoHash().toTorrentID());
    return true;
}

bool MagnetDownloadTaskImpl::GetProgressInfo(DownloadProgressInfo* info) {
    TorrentDownloadProgressInfo * magnet_progress_info = static_cast<TorrentDownloadProgressInfo *>(info);
    if (!magnet_progress_info) {
        return false;
    }

    if (!torrent_handle_) {
        const BitTorrent::MagnetUri magnet_uri(param_.url);
        if (!magnet_uri.isValid()) {
            return false;
        }

//        QString name = magnet_uri.name();
//        if (torrent_info_.isValid()) {
//            name = torrent_info_.name();
//        }

//        if (torrent_helper_.HasRoorFolder()) {
//            torrent_helper_.QStringToUtf8Buf(torrent_helper_.GetRoorFolder().toString(), info->file_name);
//        } else {
//            torrent_helper_.QStringToUtf8Buf(name, info->file_name);
//        }

        qDebug() << "torrent handle is null, id is " << param_.id;
        return false;
    }

    torrent_helper_.GetProgressInfo(torrent_handle_, &magnet_progress_info);

    return true;
}

int MagnetDownloadTaskImpl::GetTorrentFileCount() {
    if (torrent_handle_) {
        BitTorrent::TorrentInfo info = torrent_handle_->info();
        return info.filesCount();
    }
    return 0;
}

FileBaseInfo MagnetDownloadTaskImpl::GetTorrentFileInfoByIndex(int index) {
    FileBaseInfo file_base_info;
    if (!torrent_handle_) {
        return file_base_info;
    }

    BitTorrent::TorrentInfo torrent_info = torrent_handle_->info();
    if (!torrent_info.isValid()) {
        qDebug() << "ParseSource:: torrent_info_ is invalid, id is " << param_.id;
        return file_base_info;
    }
    file_base_info = torrent_helper_.GetFileBaseInfo(torrent_info_, index);
    return file_base_info;
}

FileBaseInfo MagnetDownloadTaskImpl::GetTorrentFileBaseInfo() {
    FileBaseInfo base_info;
    do {
        const BitTorrent::MagnetUri magnet_uri(param_.url);
        if (!magnet_uri.isValid()) {
            break;
        }


        QString file_name = param_.file_name;
        if (torrent_info_.isValid()) {
            if (!file_name.isEmpty()) {
                torrent_helper_.QStringToUtf8Buf(file_name, base_info.file_name);
            } else if (torrent_helper_.HasRoorFolder()) {
                torrent_helper_.QStringToUtf8Buf(torrent_helper_.GetRoorFolder().toString(), base_info.file_name);
            } else {
                torrent_helper_.QStringToUtf8Buf(torrent_info_.name(), base_info.file_name);
            }
            base_info.file_size = torrent_info_.totalSize();
            break;
         }

        QString name = magnet_uri.name();
        if (!file_name.isEmpty()) {
           torrent_helper_.QStringToUtf8Buf(file_name, base_info.file_name);
        } else if (name.isEmpty()) {
            torrent_helper_.QStringToUtf8Buf(magnet_uri.infoHash().toTorrentID().toString(), base_info.file_name);
        } else {
            torrent_helper_.QStringToUtf8Buf(name, base_info.file_name);
        }


    } while(0);
    
    return base_info;
}

void MagnetDownloadTaskImpl::SetParseCallback(void* context, PfnParse call_back) {
    // load metadata
    const BitTorrent::MagnetUri magnetUri(param_.url);
    context_ = context;
    callback_ = call_back;
    meta_data_->SetTorrentHash(magnetUri.infoHash());
    meta_data_->SetParseCallback(std::bind(&MagnetDownloadTaskImpl::OnParseMangetResult, this, std::placeholders::_1));
}

DownloadErrorCode MagnetDownloadTaskImpl::CheckCondition(const char * source) {
    DownloadErrorCode err = DownloadErrorCode::kNormal;
    do {
        const BitTorrent::MagnetUri magnetUri(QString::fromUtf8(source));
        if (!magnetUri.isValid()) {
            qDebug() << "CheckCondition: " << magnetUri.url() << " is not valid url";
            err = DownloadErrorCode::kMagnetLinkInvalid;
            break;
        }

        BitTorrent::InfoHash hash = magnetUri.infoHash();
        if (BitTorrent::Session::instance()->isKnownTorrent(hash)) {
            BitTorrent::Torrent *const torrent = BitTorrent::Session::instance()->findTorrent(hash);
            if (torrent) {
                if (torrent->isPrivate()) {
                }
                else {
                    torrent->addTrackers(magnetUri.trackers());
                    torrent->addUrlSeeds(magnetUri.urlSeeds());
                }
            }
        }

    } while(0);

    return err;
}

bool MagnetDownloadTaskImpl::IsValid(const char * source) {
    const BitTorrent::MagnetUri magnetUri(QString::fromUtf8(source));
    if (!magnetUri.isValid()) {
        return false;
    }
    return true;
}

char * MagnetDownloadTaskImpl::GetSeedingInfo() {
    char * buf = NULL;
    if (!torrent_handle_) {
        return buf;
    }
    return torrent_helper_.GetSeedingInfo(torrent_handle_);
}

void MagnetDownloadTaskImpl::ReleaseBuf(const char * buf) {
    torrent_helper_.ReleaseBuf(buf);
}


bool MagnetDownloadTaskImpl::Valid(bool is_finished) {
    return torrent_helper_.Valid(torrent_handle_, is_finished);
}

char * MagnetDownloadTaskImpl::GetTorrentList() {
    if (!torrent_info_.isValid()) {
        return NULL;
    }

    return torrent_helper_.GetTorrentList(torrent_info_, torrent_handle_);
}

void MagnetDownloadTaskImpl::OnParseMangetResult(const BitTorrent::TorrentInfo &torrent_info) {
    if (callback_) {
        torrent_info_ = torrent_info;
        if (!torrent_handle_) {
            torrent_handle_ = GetTorrentHandle();
        }
        torrent_helper_.SetRootFolder(torrent_info);
        char * file_list = torrent_helper_.GetTorrentList(torrent_info_, torrent_handle_);
        callback_(context_, file_list);
    }
}

const char * MagnetDownloadTaskImpl::GetTaskHash() {
    const BitTorrent::MagnetUri magnetUri(param_.url);
    if (!magnetUri.isValid()) {
        return NULL;
    }

    QString hash = magnetUri.infoHash().toTorrentID().toString();
    return torrent_helper_.GetTorrentHash(hash);
}

int MagnetDownloadTaskImpl::GetTorrentShareRatio() {
     return torrent_helper_.GetTorrentShareRatio(torrent_handle_);
}

int64_t MagnetDownloadTaskImpl::GetSeedingTime() {
    return torrent_helper_.GetSeedingTime(torrent_handle_);
}

int MagnetDownloadTaskImpl::GetUploadRate() {
    return torrent_helper_.GetUploadRate(torrent_handle_);
}

BitTorrent::Torrent * MagnetDownloadTaskImpl::GetTorrentHandle() {
    BitTorrent::Torrent * torrent = nullptr;

    do {
        const BitTorrent::MagnetUri magnetUri(param_.url);
        if (!magnetUri.isValid()) {
            qDebug() << "GetTorrentHandle: " << magnetUri.url() << " is not valid url";
            break;
        }

        BitTorrent::InfoHash hash = magnetUri.infoHash();
        if (BitTorrent::Session::instance()->isKnownTorrent(hash)) {
            torrent = BitTorrent::Session::instance()->findTorrent(hash);
        }

    } while(0);

    return torrent;
}

void MagnetDownloadTaskImpl::SetTorrentAddedCallback(void* context, PfnTorrentAdded callback) {
    const BitTorrent::MagnetUri magnetUri(param_.url);
    context_ = context;
    torrent_added_callback_ = callback;
    meta_data_->SetTorrentHash(magnetUri.infoHash());
    meta_data_->SetTorrentAddedCallback(std::bind(&MagnetDownloadTaskImpl::OnTorrentAdded, this, std::placeholders::_1));
}

bool MagnetDownloadTaskImpl::IsChecking() {
   return torrent_helper_.IsChecking(torrent_handle_);
}

bool MagnetDownloadTaskImpl::IsDownloading() {
    return torrent_helper_.IsDownloading(torrent_handle_);
}

void MagnetDownloadTaskImpl::OnTorrentAdded(BitTorrent::Torrent *const torrent) {
    if (!torrent_handle_) {
        torrent_handle_ = torrent;
    }

    if (torrent_added_callback_) {
        torrent_added_callback_(context_);
    }
}
