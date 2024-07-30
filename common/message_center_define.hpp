
#ifndef NAS_COMMON_MESSAGE_CENTER_DEFINE_HPP_
#define NAS_COMMON_MESSAGE_CENTER_DEFINE_HPP_

namespace common {
namespace message_type {
static const char* kAllType = "nas.all";
static const char* kNasFileStation = "nas.file_station";
static const char* kNasUpdate = "nas.update";
static const char* kNasDownload = "nas.download";
static const char* kNasAlbum = "nas.album";
static const char* kNasMovie = "nas.movie";
static const char* kCentreServer = "centre_server";
} // namespace message_type
namespace message_action {
static const char* kMoveFile = "move_file";
static const char* kCopyFile = "copy_file";
static const char* kDeleteFile = "delete_file";
static const char* kDownloadFile = "download_file";
static const char* kPackFile = "pack_file";
static const char* kUnpackFile = "unpack_file";
static const char* kUpdate = "update";
static const char* kInstall = "install";
static const char* kNotify = "notify";
}  // namespace message_action

namespace message_status {
static const char* kSuccess = "success";
static const char* kPartSuccess = "part_success";
static const char* kFailed = "failed";
static const char* kStart = "start_work";
static const char* kFinish = "finish_work";
static const char* kFound = "found";
static const char* kNotFound = "not_found";
static const char* kOffline = "offline";
}  // namespace message_action
namespace message_template {

// 设备离线模板s
static const int kWsOfflineTempateId = 301;
} // namespace message_template
}  // namespace common
#endif  // NAS_COMMON_MESSAGE_CENTER_DEFINE_HPP_