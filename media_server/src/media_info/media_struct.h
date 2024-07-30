// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/01/18 15:36
#ifndef NAS_MEDIA_SERVER_SRC_MEDIA_INFO_MEDIA_STRUCT_H_
#define NAS_MEDIA_SERVER_SRC_MEDIA_INFO_MEDIA_STRUCT_H_
#include <map>
#include <memory>
#include <string>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"

#include "path/file_path_unit.h"

namespace nas {
struct MediaMetadata : public std::map<std::string, std::string> {
  void ToJsonObj(base::Value* json_obj) {
    base::Value json_metadata(base::Value::Type::DICT);
    for (const auto& val : *this) {
      base::Value single_metadata(base::Value::Type::DICT);
      json_metadata.SetStringKey(val.first, val.second);
    }
    json_obj->SetKey("metadata", std::move(json_metadata));
  }
  bool FromJson(const base::Value& json_obj) {
    auto json_dict = json_obj.GetIfDict();
    if (!json_dict) {
      return false;
    }

    for (auto itr = json_dict->begin(); itr != json_dict->end(); itr++) {
      auto key = itr->first;
      auto value = itr->second.GetIfString();
      if (key.empty() || !value) {
        continue;
      }
      insert(std::make_pair(key, *value));
    }

    return true;
  }
};

struct MediaBasicInfo {
  base::FilePath local_path;
  std::string title;
  int64_t file_size = 0;
  std::string media_container;  // 视频容器,即封装格式
  int64_t duration_ms = 0;
  int width = 0;
  int height = 0;
  int total_bitrate = 0;
  bool has_subtitle = false;
  MediaMetadata metadata;

  bool ToJsonObj(base::Value* json_base_info) {
    json_base_info->SetStringKey("path", local_path.AsUTF8Unsafe());
    json_base_info->SetStringKey("title", title);
    json_base_info->SetStringKey("size", std::to_string(file_size));
    json_base_info->SetStringKey("container", media_container);
    json_base_info->SetStringKey("duration", std::to_string(duration_ms));
    json_base_info->SetStringKey("width", std::to_string(width));
    json_base_info->SetStringKey("height", std::to_string(height));
    json_base_info->SetStringKey("total_bitrate",
                                 std::to_string(total_bitrate));
    json_base_info->SetBoolKey("has_subtitle", has_subtitle);

    metadata.ToJsonObj(json_base_info);
    return true;
  }

  bool FromJsonObj(const base::Value& json_obj) {
    auto container_ptr = json_obj.FindStringKey("container");
    if (container_ptr) {
      media_container = *container_ptr;
    }
    auto path_ptr = json_obj.FindStringKey("path");
    if (path_ptr) {
      local_path = path_unit::BaseFilePathFromU8(*path_ptr);
    }
    auto title_ptr = json_obj.FindStringKey("title");
    if (title_ptr) {
      title = *title_ptr;
    }

    auto size_ptr = json_obj.FindStringKey("size");
    if (size_ptr) {
      base::StringToInt64(*size_ptr, &file_size);
    }
    auto duration_ptr = json_obj.FindStringKey("duration");
    if (duration_ptr) {
      base::StringToInt64(*duration_ptr, &duration_ms);
    }
    auto width_ptr = json_obj.FindStringKey("width");
    if (width_ptr) {
      base::StringToInt(*width_ptr, &width);
    }
    auto height_ptr = json_obj.FindStringKey("height");
    if (height_ptr) {
      base::StringToInt(*height_ptr, &height);
    }
    auto total_bitrate_ptr = json_obj.FindStringKey("total_bitrate");
    if (total_bitrate_ptr) {
      base::StringToInt(*total_bitrate_ptr, &total_bitrate);
    }

    auto has_subtitle_ptr = json_obj.FindBoolKey("has_subtitle");
    if (has_subtitle_ptr.has_value()) {
      has_subtitle = has_subtitle_ptr.value();
    }

    auto json_metadata = json_obj.FindListKey("metadata");
    if (json_metadata) {
      metadata.FromJson(*json_metadata);
    }
    return true;
  }
};

using MediaBasicInfoPtr = std::shared_ptr<MediaBasicInfo>;

struct MediaStream {
  int stream_index = 0;
  int codec_id = 0;
  std::string codec_name;
  std::string codec_long_name;
  std::string title;
  std::string language;
  int bitrate = 0;
  MediaMetadata metadata;

  void ParseMetadata() {
    for (const auto& val : metadata) {
      if (val.first.find("BPS") != -1 && bitrate == 0) {
        base::StringToInt(val.second, &bitrate);
      } else if (val.first.find("language") != -1) {
        language = val.second;
      } else if (val.first.find("title") != -1) {
        title = val.second;
      }
    }
  }

  bool ToJsonObj(base::Value* json_obj) {
    json_obj->SetIntKey("stream_index", stream_index);
    json_obj->SetIntKey("codec_id", codec_id);
    json_obj->SetStringKey("codec_name", codec_name);
    json_obj->SetStringKey("codec_long_name", codec_long_name);
    json_obj->SetStringKey("title", title);
    json_obj->SetStringKey("language", language);
    json_obj->SetIntKey("bitrate", bitrate);
    metadata.ToJsonObj(json_obj);
    return true;
  }

  bool FromJsonObj(const base::Value& json_obj) {
    auto stream_index_ptr = json_obj.FindIntKey("stream_index");
    if (stream_index_ptr) {
      stream_index = *stream_index_ptr;
    }
    auto codec_id_ptr = json_obj.FindIntKey("codec_id");
    if (codec_id_ptr) {
      codec_id = *codec_id_ptr;
    }

    auto codec_name_ptr = json_obj.FindStringKey("codec_name");
    if (codec_name_ptr) {
      codec_name = *codec_name_ptr;
    }

    auto codec_long_name_ptr = json_obj.FindStringKey("codec_long_name");
    if (codec_long_name_ptr) {
      codec_long_name = *codec_long_name_ptr;
    }

    auto title_ptr = json_obj.FindStringKey("title");
    if (title_ptr) {
      title = *title_ptr;
    }

    auto language_ptr = json_obj.FindStringKey("language");
    if (language_ptr) {
      language = *language_ptr;
    }

    auto bitrate_ptr = json_obj.FindIntKey("bitrate");
    if (bitrate_ptr) {
      bitrate = *bitrate_ptr;
    }

    auto json_metadata = json_obj.FindListKey("metadata");
    if (json_metadata) {
      metadata.FromJson(*json_metadata);
    }
    return true;
  }
};

struct HDRInfo {
  int color_primaries = 0;
  int color_trc = 0;
  int color_space = 0;
  bool ToJsonObj(base::Value* json_obj) {
    base::Value hdr_obj(base::Value::Type::DICTIONARY);
    hdr_obj.SetIntKey("color_primaries", color_primaries);
    hdr_obj.SetIntKey("color_trc", color_trc);
    hdr_obj.SetIntKey("color_space", color_space);
    json_obj->SetKey("hdr", std::move(hdr_obj));
    return true;
  }
  bool FromJsonObj(const base::Value& json_obj) {
    auto hdr_obj_ptr = json_obj.FindDictKey("hdr");
    if (!hdr_obj_ptr) {
      return false;
    }

    auto color_primaries_ptr = hdr_obj_ptr->FindIntKey("color_primaries");
    auto color_trc_ptr = hdr_obj_ptr->FindIntKey("color_trc");
    auto color_space_ptr = hdr_obj_ptr->FindIntKey("color_space");
    if (color_primaries_ptr) {
      color_primaries = *color_primaries_ptr;
    }
    if (color_trc_ptr) {
      color_trc = *color_trc_ptr;
    }
    if (color_space_ptr) {
      color_space = *color_space_ptr;
    }

    return true;
  }
};

struct VideoStream : public MediaStream {
  std::string profile;
  int level = 0;
  std::string aspect_ratio;
  double real_frame_bitrate = 0;
  double avg_frame_bitrate = 0;
  int rotate = 0;
  std::shared_ptr<HDRInfo> hdr_info;
  bool ToJsonObj(base::Value* json_obj) {
    MediaStream::ToJsonObj(json_obj);
    json_obj->SetStringKey("profile", profile);
    json_obj->SetIntKey("level", level);
    json_obj->SetStringKey("aspect_ratio", aspect_ratio);
    json_obj->SetDoubleKey("real_frame_bitrate", real_frame_bitrate);
    json_obj->SetDoubleKey("avg_frame_bitrate", avg_frame_bitrate);
    json_obj->SetIntKey("rotate", rotate);
    if (hdr_info) {
      hdr_info->ToJsonObj(json_obj);
    }
    return true;
  }
  bool FromJsonObj(const base::Value& json_obj) {
    MediaStream::FromJsonObj(json_obj);
    auto profile_ptr = json_obj.FindStringKey("profile");
    if (profile_ptr) {
      profile = *profile_ptr;
    }

    auto level_ptr = json_obj.FindIntKey("level");
    if (level_ptr) {
      level = *level_ptr;
    }
    auto aspect_ratio_ptr = json_obj.FindStringKey("aspect_ratio");
    if (aspect_ratio_ptr) {
      aspect_ratio = *aspect_ratio_ptr;
    }

    auto real_frame_bitrate_ptr = json_obj.FindDoubleKey("real_frame_bitrate");
    if (real_frame_bitrate_ptr) {
      real_frame_bitrate = *real_frame_bitrate_ptr;
    }

    auto avg_frame_bitrate_ptr = json_obj.FindDoubleKey("avg_frame_bitrate");
    if (avg_frame_bitrate_ptr) {
      avg_frame_bitrate = *avg_frame_bitrate_ptr;
    }
    auto rotate_ptr = json_obj.FindIntKey("rotate");
    if (rotate_ptr) {
      rotate = *rotate_ptr;
    }

    HDRInfo hdr;
    if (hdr.FromJsonObj(json_obj)) {
      hdr_info = std::make_shared<HDRInfo>(hdr);
    }

    return true;
  }
};
using VideoStreamPtr = std::shared_ptr<VideoStream>;

struct AudioStream : public MediaStream {
  int channel = 0;
  int sample_rate = 0;
  bool ToJsonObj(base::Value* json_obj) {
    MediaStream::ToJsonObj(json_obj);
    json_obj->SetIntKey("channel", channel);
    json_obj->SetIntKey("sample_rate", sample_rate);
    return true;
  }
  bool FromJsonObj(const base::Value& json_obj) {
    MediaStream::FromJsonObj(json_obj);
    auto channel_ptr = json_obj.FindIntKey("channel");
    if (channel_ptr) {
      channel = *channel_ptr;
    }

    auto sample_rate_ptr = json_obj.FindIntKey("sample_rate");
    if (sample_rate_ptr) {
      sample_rate = *sample_rate_ptr;
    }

    return true;
  }
};
using AudioStreamPtr = std::shared_ptr<AudioStream>;

struct SubtitleStream : public MediaStream {
  base::FilePath vtt_file_path;
  bool ToJsonObj(base::Value* json_obj) {
    MediaStream::ToJsonObj(json_obj);
    json_obj->SetStringKey("vtt_file_path", vtt_file_path.AsUTF8Unsafe());
    return true;
  }
  bool FromJsonObj(const base::Value& json_obj) {
    MediaStream::FromJsonObj(json_obj);
    auto vtt_file_path_ptr = json_obj.FindStringKey("vtt_file_path");
    if (vtt_file_path_ptr) {
      vtt_file_path = path_unit::BaseFilePathFromU8(*vtt_file_path_ptr);
    }

    return true;
  }
};
using SubtitleStreamPtr = std::shared_ptr<SubtitleStream>;

struct MediaChapter {
  int stream_index = 0;
  std::string title;
  double start_time_sec = 0.0;
  double end_time_sec = 0.0;
  MediaMetadata metadata;
  bool ToJsonObj(base::Value* json_obj) {
    json_obj->SetIntKey("stream_index", stream_index);
    json_obj->SetStringKey("title", title);
    json_obj->SetDoubleKey("start_time_sec", start_time_sec);
    json_obj->SetDoubleKey("end_time_sec", end_time_sec);
    metadata.ToJsonObj(json_obj);
    return true;
  }

  bool FromJsonObj(const base::Value& json_obj) {
    auto stream_index_ptr = json_obj.FindIntKey("stream_index");
    if (stream_index_ptr) {
      stream_index = *stream_index_ptr;
    }

    auto title_ptr = json_obj.FindStringKey("title");
    if (title_ptr) {
      title = *title_ptr;
    }

    auto start_time_sec_ptr = json_obj.FindDoubleKey("start_time_sec");
    if (start_time_sec_ptr) {
      start_time_sec = *start_time_sec_ptr;
    }
    auto end_time_sec_ptr = json_obj.FindDoubleKey("end_time_sec");
    if (end_time_sec_ptr) {
      end_time_sec = *end_time_sec_ptr;
    }

    auto json_metadata = json_obj.FindListKey("metadata");
    if (json_metadata) {
      metadata.FromJson(*json_metadata);
    }
    return true;
  }
};
using MediaChapterPtr = std::shared_ptr<MediaChapter>;

struct MediaAttachment {
  int stream_index = 0;
  int codec_id = 0;
  std::string codec_name;
  std::string codec_long_name;
  std::string file_name;
  std::string mime_type;
  MediaMetadata metadata;
  void ParseMetadata() {
    for (const auto& val : metadata) {
      if (val.first.find("filename") != -1) {
        file_name = val.second;
      } else if (val.first.find("mimetype") != -1) {
        mime_type = val.second;
      }
    }
  }
  bool ToJsonObj(base::Value* json_obj) {
    json_obj->SetIntKey("stream_index", stream_index);
    json_obj->SetIntKey("codec_id", codec_id);
    json_obj->SetStringKey("codec_name", codec_name);
    json_obj->SetStringKey("codec_long_name", codec_long_name);
    json_obj->SetStringKey("file_name", file_name);
    json_obj->SetStringKey("mime_type", mime_type);
    metadata.ToJsonObj(json_obj);
    return true;
  }
  bool FromJsonObj(const base::Value& json_obj) {
    auto stream_index_ptr = json_obj.FindIntKey("stream_index");
    if (stream_index_ptr) {
      stream_index = *stream_index_ptr;
    }

    auto codec_id_ptr = json_obj.FindIntKey("codec_id");
    if (codec_id_ptr) {
      codec_id = *codec_id_ptr;
    }

    auto codec_name_ptr = json_obj.FindStringKey("codec_name");
    if (codec_name_ptr) {
      codec_name = *codec_name_ptr;
    }

    auto codec_long_name_ptr = json_obj.FindStringKey("codec_long_name");
    if (codec_long_name_ptr) {
      codec_long_name = *codec_long_name_ptr;
    }
    auto file_name_ptr = json_obj.FindStringKey("file_name");
    if (file_name_ptr) {
      file_name = *file_name_ptr;
    }
    auto mime_type_ptr = json_obj.FindStringKey("mime_type");
    if (mime_type_ptr) {
      mime_type = *mime_type_ptr;
    }

    auto json_metadata = json_obj.FindListKey("metadata");
    if (json_metadata) {
      metadata.FromJson(*json_metadata);
    }
    return true;
  }
};
using MediaAttachmentPtr = std::shared_ptr<MediaAttachment>;

}  // namespace nas

#endif  // NAS_MEDIA_SERVER_SRC_MEDIA_INFO_MEDIA_STRUCT_H_
