

#ifndef NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_DATA_DEFINE_H_
#define NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_DATA_DEFINE_H_

#include <memory>
#include <vector>

namespace nas {
enum HwAccels {
  kUnKnow,
  kIntelQsv,
  kNvidiaCuda,
  kGeneralDxva2,
  kVaapi,
};

using HwAccelsListPtr = std::shared_ptr<std::vector<HwAccels>>;
}  // namespace nas

#endif  // NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_DATA_DEFINE_H_
