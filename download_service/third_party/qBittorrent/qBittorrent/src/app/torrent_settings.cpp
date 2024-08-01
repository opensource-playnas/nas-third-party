#include "torrent_settings.h"

#include "base/bittorrent/session.h"
#include "base/net/portforwarder.h"

TorrentSettings::TorrentSettings() {

}

TorrentSettings::~TorrentSettings() {

}

// **************************设置项****************************
void TorrentSettings::EnableCreateSubdir(bool enable) {
    BitTorrent::TorrentContentLayout layout = enable ? BitTorrent::TorrentContentLayout::Subfolder:  BitTorrent::TorrentContentLayout::Original;
    BitTorrent::Session::instance()->setTorrentContentLayout(layout);
}

void TorrentSettings::EnableAutoDownload(bool enable) {
    BitTorrent::Session::instance()->setAddTorrentPaused(!enable);
}

void TorrentSettings::SetTcpListenPort(int port) {
    BitTorrent::Session::instance()->setPort(port);
}

void TorrentSettings::SetMaxUploadSpeed(int64_t speed) {
    BitTorrent::Session::instance()->setGlobalUploadSpeedLimit(speed*1024);
}

void TorrentSettings::SetMaxDownloadSpeed(int64_t speed) {
    BitTorrent::Session::instance()->setGlobalDownloadSpeedLimit(speed*1024);
}

void TorrentSettings::SetEncryptionMode(EncryptionMode mode) {
    BitTorrent::Session::instance()->setEncryption(mode);
}

void TorrentSettings::SetMaxConnections(int connections){
    BitTorrent::Session::instance()->setMaxConnections(connections);
}

void TorrentSettings::SetTorrentMaxConnections(int connections){
    BitTorrent::Session::instance()->setMaxConnectionsPerTorrent(connections);
}

void TorrentSettings::EnableDHT(bool enable) {
    BitTorrent::Session::instance()->setDHTEnabled(enable);
}

void TorrentSettings::EnableUPnPAndNat(bool enable) {
    Net::PortForwarder::instance()->setEnabled(enable);
}

void TorrentSettings::SetPreallocationEnabled(bool enable) {
    BitTorrent::Session::instance()->setPreallocationEnabled(enable);
}

