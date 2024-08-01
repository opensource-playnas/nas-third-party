#include "subscibe_event.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include "base/bittorrent/torrent.h"
#include "base/bittorrent/torrentinfo.h"
#include "base/bittorrent/session.h"
#include "torrent_helper.h"

Q_DECLARE_METATYPE(const BitTorrent::Torrent *)
Q_DECLARE_METATYPE(const BitTorrent::TorrentInfo *)

SubscribeEvent::SubscribeEvent() {
    qRegisterMetaType<const BitTorrent::Torrent *>("const BitTorrent::Torrent *");
    qRegisterMetaType<const BitTorrent::TorrentInfo *>("const BitTorrent::TorrentInfo *");
}

void SubscribeEvent::SetTorrentHash(const BitTorrent::InfoHash& hash) {
      hash_ = hash;
}

void SubscribeEvent::SetParseCallback(PfnParseFinishedCallback call_back) {
   parse_call_back_ = call_back;
   bool ret = connect(BitTorrent::Session::instance(), &BitTorrent::Session::metadataDownloaded, this, &SubscribeEvent::OnMetaDataLoadCompleted);
   qDebug() << "connect ret " << ret;
}

void SubscribeEvent::OnMetaDataLoadCompleted(const BitTorrent::TorrentInfo &torrent_info) {
    if (hash_ != torrent_info.infoHash() ) {
        return;
    }

    disconnect(BitTorrent::Session::instance(), &BitTorrent::Session::metadataDownloaded, this, &SubscribeEvent::OnMetaDataLoadCompleted);

    if (parse_call_back_) {
        parse_call_back_(torrent_info);
    }
}

void SubscribeEvent::SetTorrentAddedCallback(PfnTorrentAddedCallback callback) {
    torrent_added_callback_ = callback;
    connect(BitTorrent::Session::instance(), &BitTorrent::Session::torrentAdded, this, &SubscribeEvent::OnTorrentAdded);
}

void SubscribeEvent::OnTorrentAdded(BitTorrent::Torrent *const torrent) {
    if (hash_ !=  torrent->infoHash()) {
        return;
    }

    disconnect(BitTorrent::Session::instance(), &BitTorrent::Session::torrentAdded, this, &SubscribeEvent::OnTorrentAdded);

    if (torrent_added_callback_) {
        torrent_added_callback_(torrent);
    }
}
