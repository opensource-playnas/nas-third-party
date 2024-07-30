// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/01/30 20:03

#ifndef NAS_DLNA_SERVER_SRC_PUBLIC_DEFINE_H_
#define NAS_DLNA_SERVER_SRC_PUBLIC_DEFINE_H_

#include <map>
#include <string>
#include <vector>
namespace nas {
extern const char* kNormalThreadName;
extern const char* kScreenCastingMgrThreadName;
extern const std::map<uint32_t, const char*> kDlnaErrorList;

enum class DlnaServiceErrorNo : unsigned int {
  kSuccess,
  kParamInvalid,
  kCreateScreenCastingFailed,
  kScreenCastingDeviceNotExist,
  kPlayUrlInvalid,
  kScreenCastingDeviceNoneProtocol,
  kNonsupportMimeTypeExtension,
  kScreenCastingDeviceNonsupportProtocol,
  kNginxPortInvalid,
};

struct NginxMimeTypes {
  std::string mime_type;
  std::vector<std::string> file_extsion;
};

static const std::vector<NginxMimeTypes> kMimeTypes = {
    /*  {"image/gif", {"gif"}},
      {"image/jpeg", {"jpeg", "jpg", "jpe", "thm"}},
      {"image/jp2", {"jp2"}},
      {"image/avif", {"avif"}},
      {"image/png", {"png"}},
      {"image/svg+xml", {"svg", "svgz"}},
      {"image/tiff", {"tif", "tiff"}},
      {"image/vnd.wap.wbmp", {"wbmp"}},
      {"image/webp", {"webp"}},
      {"image/x-icon", {"ico"}},
      {"image/x-jng", {"jng"}},
      {"image/x-ms-bmp", {"bmp"}},
      {"audio/midi", {"mid", "midi", "kar"}},
      {"audio/mpeg", {"mp3", "mpa", "mp2"}},
      {"audio/mp4", {"m4a"}},
      {"audio/x-ms-wma", {"wma"}},
      {"audio/x-wav", {"wav"}},
      {"audio/ogg", {"ogg"}},
      {"audio/x-realaudio", {"ra"}},
      {"audio/x-aiff", {"aif", "aifc", "aiff"}},*/
    {"video/mpeg", {"mpeg", "mpg"}},
    {"video/mp4", {"mp4", "m4v"}},
    {"video/MP2T", {"ts"}},
    {"video/x-ms-wmv", {"wmv", "wtv"}},
    {"video/x-ms-asf", {"asx", "asf"}},
    {"video/x-matroska", {"mkv"}},
    {"video/x-flv", {"flv"}},
    {"video/x-msvideo", {"avi", "divx", "xvid"}},
    {"video/3gpp", {"3gpp", "3gp"}},
    {"video/quicktime", {"mov"}},
    {"video/webm", {"webm"}},
    {"video/x-mng", {"mng"}}};

}  // namespace nas

#endif  // NAS_DLNA_SERVER_SRC_PUBLIC_DEFINE_H_
