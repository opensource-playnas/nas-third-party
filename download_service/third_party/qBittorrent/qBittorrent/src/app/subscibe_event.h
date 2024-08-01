#ifndef SUBSCRIBEEVENT_H
#define SUBSCRIBEEVENT_H

#include <functional>
#include <QObject>
#include "base/bittorrent/torrent.h"
#include "base/bittorrent/infohash.h"
#include "torrent_task_public_define.h"

using PfnParseFinishedCallback = std::function<void(const BitTorrent::TorrentInfo &torrent_info)>;
using PfnTorrentAddedCallback = std::function<void(BitTorrent::Torrent *const torrent)>;

class SubscribeEvent:public QObject
{
    Q_OBJECT
public:
    SubscribeEvent();
    void SetTorrentHash(const BitTorrent::InfoHash& hash);
    // 设置解析回调
    void SetParseCallback(PfnParseFinishedCallback call_back);
    // 设置种子添加回调
    void SetTorrentAddedCallback(PfnTorrentAddedCallback callback);

public slots:
    void OnMetaDataLoadCompleted(const BitTorrent::TorrentInfo &torrent_info);
    void OnTorrentAdded(BitTorrent::Torrent *const torrent);

private:
    PfnParseFinishedCallback parse_call_back_ = NULL;
    BitTorrent::InfoHash hash_;
    PfnTorrentAddedCallback torrent_added_callback_ = NULL;
};

#endif // SUBSCRIBEEVENT_H
