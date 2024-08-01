#ifndef MagnetDownloadTaskImpl_H
#define MagnetDownloadTaskImpl_H


#include "torrent_task_public_define.h"

#include <string>
#include <mutex>
#include "base/bittorrent/torrent.h"
#include "base/bittorrent/infohash.h"
#include "base/bittorrent/torrentinfo.h"
#include "base/bittorrent/magneturi.h"
#include "base/bittorrent/session.h"

#include "subscibe_event.h"
#include "torrent_helper.h"

class MagnetDownloadTaskImpl : public MagnetDownloadTask {
public:
    MagnetDownloadTaskImpl();
    ~MagnetDownloadTaskImpl() = default;

public:
    DownloadErrorCode Start() override;
    bool Pause() override;
    bool Resume(bool is_failed) override;
    bool Cancel(bool is_delete_local_file) override;
    void SetParam(const TorrentDownloadParam& param) override;
    bool GetProgressInfo(DownloadProgressInfo* info) override;
    int GetTorrentFileCount() override;
    FileBaseInfo GetTorrentFileInfoByIndex(int index) override;
    FileBaseInfo GetTorrentFileBaseInfo() override;
    DownloadErrorCode CheckCondition(const char * source) override;
    void SetParseCallback(void* context, PfnParse call_back) override;
    bool IsValid(const char * source) override;
    char * GetSeedingInfo() override;
    void ReleaseBuf(const char * buf) override;
    bool Valid(bool is_finished) override;
    char * GetTorrentList() override;
    const char * GetTaskHash() override;
    int GetTorrentShareRatio() override;
    int64_t GetSeedingTime() override;
    int GetUploadRate() override;
    void SetTorrentAddedCallback(void* context, PfnTorrentAdded callback) override;
    bool IsChecking() override;
    bool IsDownloading() override;

private:
    BitTorrent::Torrent * GetTorrentHandle();
    void OnParseMangetResult(const BitTorrent::TorrentInfo &torrent_info);
    void OnTorrentAdded(BitTorrent::Torrent *const torrent);

private:
    TorrentParam param_;
    BitTorrent::TorrentInfo torrent_info_;
    BitTorrent::Torrent* torrent_handle_ = nullptr;
    SubscribeEvent * meta_data_ = nullptr;
    TorrentHelper torrent_helper_;
    void* context_ = NULL;
    PfnParse callback_;
    PfnTorrentAdded torrent_added_callback_ = nullptr;
};

#endif // MagnetDownloadTaskImpl_H
