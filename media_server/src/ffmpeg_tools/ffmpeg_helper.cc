// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/02/14 15:28

#include "ffmpeg_helper.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

#include "base/files/file_util.h"
#include "base/json/json_writer.h"

#include "libavutil/imgutils.h"

typedef struct DecodeContext {
  AVBufferRef* hw_device_ref;
} DecodeContext;

namespace nas {

FFmpegHelper* FFmpegHelper::GetInstance() {
  return base::Singleton<FFmpegHelper>::get();
}

FFmpegHelper::FFmpegHelper() {}

void FFmpegHelper::RunCommand() {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (!cmd_line) {
    return;
  }
  if (cmd_line->HasSwitch("init")) {
    if (!HandleInit()) {
      LOG(INFO) << "HandleParse failed";
      return;
    }
  } else if (cmd_line->HasSwitch("parse")) {
    base::FilePath media_path = cmd_line->GetSwitchValuePath("parse");
    if (!base::PathExists(media_path)) {
      LOG(INFO) << "media path not exist: " << media_path.AsUTF8Unsafe();
      return;
    }

    if (!HandleParse(media_path)) {
      LOG(INFO) << "HandleParse failed ";
      return;
    }
  }
}

FFmpegHelper::~FFmpegHelper() {}

void FFmpegHelper::InitHWAccels() {
  if (NVIDIACuda()) {
    hw_accels_.push_back(kNvidiaCuda);
    LOG(INFO) << "hw accles:" << kNvidiaCuda;
  }

#if BUILDFLAG(IS_WIN)
  if (IntelQsv()) {
    hw_accels_.push_back(kIntelQsv);
    LOG(INFO) << "hw accles:" << kIntelQsv;
  }
  if (GeneralDxva2()) {
    hw_accels_.push_back(kGeneralDxva2);
    LOG(INFO) << "hw accles:" << kGeneralDxva2;
  }
#else
  if (Vaapi()) {
    hw_accels_.push_back(kVaapi);
    LOG(INFO) << "hw accles:" << kVaapi;
  }
#endif
}

bool FFmpegHelper::HandleInit() {
  if (!Init()) {
    LOG(INFO) << "Init failed";
    return false;
  }
  LOG(INFO) << "HandleInit";
  InitHWAccels();

  // 将值写入输出文件
  base::Value json_root(base::Value::Type::DICT);
  json_root.SetStringKey("encoder", encoder_);

  if (!hw_accels_.empty()) {
    base::Value json_hw_value(base::Value::Type::LIST);
    for (auto& hw_value : hw_accels_) {
      json_hw_value.Append(hw_value);
    }
    json_root.SetKey("hw_accels", std::move(json_hw_value));
  }

  std::string output_str;
  base::JSONWriter::Write(json_root, &output_str);

  std::cout << "output:" << output_str << std::endl;
  LOG(INFO) << "End: " << output_str;
  return true;
}

bool FFmpegHelper::HandleParse(const base::FilePath& media_path) {
  LOG(INFO) << "HandleParse: " << media_path.AsUTF8Unsafe();

  if (!Init()) {
    LOG(INFO) << "Init failed";
    return false;
  }

  if (!IsVideoFile(media_path)) {
    LOG(INFO) << "media not is video ";
    return false;
  }

  auto base_info = std::make_shared<MediaBasicInfo>();
  base_info->local_path = media_path;
  base::GetFileSize(media_path, &base_info->file_size);
  base_info->title = media_path.RemoveExtension().BaseName().AsUTF8Unsafe();

  std::vector<VideoStreamPtr> video_stream;
  std::vector<AudioStreamPtr> audio_stream;
  std::vector<SubtitleStreamPtr> subtitle_stream;
  std::vector<MediaChapterPtr> chapter;
  std::vector<MediaAttachmentPtr> attachment;
  if (!ParseMediaInfo(base_info, video_stream, audio_stream, subtitle_stream,
                      chapter, attachment)) {
    LOG(INFO) << "ParseMediaInfo failed";
    return false;
  }
  // 将结果写入输出文件
  base::Value json_root(base::Value::Type::DICT);

  if (video_stream.empty()) {
    LOG(INFO) << "media video stream is empty";
    return false;
  }

  // 基本信息
  base::Value json_base_info(base::Value::Type::DICT);
  base_info->ToJsonObj(&json_base_info);

  // 视频信息
  base::Value json_video(base::Value::Type::LIST);
  for (auto& val : video_stream) {
    if (val->bitrate == 0) {
      val->bitrate = base_info->total_bitrate;
    }

    base::Value single_video(base::Value::Type::DICT);
    val->ToJsonObj(&single_video);
    json_video.Append(std::move(single_video));
  }

  // 音频信息
  base::Value json_audio(base::Value::Type::LIST);
  for (const auto& val : audio_stream) {
    base::Value single_audio(base::Value::Type::DICT);
    val->ToJsonObj(&single_audio);
    json_audio.Append(std::move(single_audio));
  }

  // 字幕信息
  base::Value json_subtitle(base::Value::Type::LIST);
  for (const auto& val : subtitle_stream) {
    base::Value single_subtitle(base::Value::Type::DICT);
    val->ToJsonObj(&single_subtitle);
    json_subtitle.Append(std::move(single_subtitle));
  }

  // 章节信息
  base::Value json_chapter(base::Value::Type::LIST);
  for (const auto& val : chapter) {
    base::Value single_chapter(base::Value::Type::DICT);
    val->ToJsonObj(&single_chapter);
    json_chapter.Append(std::move(single_chapter));
  }

  // 附加信息
  base::Value json_attachment(base::Value::Type::LIST);
  for (const auto& val : attachment) {
    base::Value single_attachment(base::Value::Type::DICT);
    val->ToJsonObj(&single_attachment);
    json_attachment.Append(std::move(single_attachment));
  }

  json_root.SetKey("base", std::move(json_base_info));
  json_root.SetKey("video", std::move(json_video));
  json_root.SetKey("audio", std::move(json_audio));
  json_root.SetKey("subtitle", std::move(json_subtitle));
  json_root.SetKey("chapter", std::move(json_chapter));
  json_root.SetKey("attachment", std::move(json_attachment));

  std::string output_str;
  base::JSONWriter::Write(json_root, &output_str);
  std::cout << "output:" << output_str;
  LOG(INFO) << "End";
  return true;
}

bool FFmpegHelper::Init() {
  init_succ_ = false;
  do {
    base::FilePath exe_dir;
    base::PathService::Get(base::DIR_EXE, &exe_dir);
    ffmpeg_dir_path_ = exe_dir.Append(FILE_PATH_LITERAL("ffmpeg"));
#if BUILDFLAG(IS_WIN)
    ffmpeg_tools_path_ =
        ffmpeg_dir_path_.Append(FILE_PATH_LITERAL("ffmpeg.exe"));
#else
    ffmpeg_tools_path_ = ffmpeg_dir_path_.Append(FILE_PATH_LITERAL("ffmpeg"));
#endif

    if (!base::PathExists(ffmpeg_tools_path_)) {
      break;
    }

    if (!InitFFmpegFuncFromAvformat() || !InitFFmpegFuncFromAvcodec() ||
        !InitFFmpegFuncFromAvutil() || !InitFFmpegFuncFromSwscale()) {
      LOG(INFO) << "init ffmpeg library failed";
      break;
    }

    if (pfn_av_log_set_level_) {
      pfn_av_log_set_level_(AV_LOG_ERROR);
    }
    init_succ_ = true;
  } while (false);
  return init_succ_;
}

bool FFmpegHelper::IsVideoFile(const base::FilePath file_path) {
  AVFormatContext* format_context_raw_ptr = nullptr;

  if (!pfn_avformat_open_input_ || !pfn_avformat_close_input_ ||
      !pfn_avformat_find_stream_info_ || !pfn_av_find_best_stream_) {
    return false;
  }

  int ret = pfn_avformat_open_input_(&format_context_raw_ptr,
                                     file_path.AsUTF8Unsafe().c_str(), nullptr,
                                     nullptr);

  if (ret < 0 || !format_context_raw_ptr) {
    LOG(ERROR) << "avformat_open_input error:" << ret
               << ", file_path:" << file_path.AsUTF8Unsafe();
    return false;
  }

  std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>>
      format_context_ptr(format_context_raw_ptr, [this](AVFormatContext* ptr) {
        pfn_avformat_close_input_(&ptr);
      });

  ret = pfn_avformat_find_stream_info_(format_context_raw_ptr, nullptr);
  if (ret != 0) {
    return false;
  }

  auto video_stream = pfn_av_find_best_stream_(
      format_context_raw_ptr, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

  if (video_stream < 0) {
    return false;
  }

  return true;
}

bool FFmpegHelper::ParseMediaInfo(
    MediaBasicInfoPtr& base_info,
    std::vector<VideoStreamPtr>& video_stream,
    std::vector<AudioStreamPtr>& audio_stream,
    std::vector<SubtitleStreamPtr>& subtitle_stream,
    std::vector<MediaChapterPtr>& chapter,
    std::vector<MediaAttachmentPtr>& attachment) {
  if (!base_info || !base::PathExists(base_info->local_path)) {
    LOG(INFO) << "media file error";
    return false;
  }

  if (!pfn_avformat_open_input_ || !pfn_avformat_close_input_ ||
      !pfn_avformat_find_stream_info_) {
    return false;
  }

  AVFormatContext* format_context_raw_ptr = nullptr;
  int ret = -1;
  std::string path_temp = base_info->local_path.AsUTF8Unsafe();
  ret = pfn_avformat_open_input_(&format_context_raw_ptr, path_temp.c_str(),
                                 nullptr, nullptr);

  if (ret < 0 || !format_context_raw_ptr) {
    LOG(ERROR) << "avformat_open_input error:" << ret;
    return false;
  }

  // 自动释放
  std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>>
      format_context_ptr(format_context_raw_ptr, [this](AVFormatContext* ptr) {
        pfn_avformat_close_input_(&ptr);
      });

  ret = pfn_avformat_find_stream_info_(format_context_raw_ptr, nullptr);
  if (ret != 0) {
    return false;
  }

  // 输出媒体信息到控制
  // pfn_av_dump_format_(format_context_raw_ptr, nullptr, path_temp.c_str(), 0);

  // 基本信息
  base_info->duration_ms = format_context_ptr->duration / 1000;
  base_info->media_container = format_context_ptr->iformat->name;
  base_info->total_bitrate = format_context_ptr->bit_rate;

  GetMetadataFromAVDictionary(format_context_ptr->metadata,
                              base_info->metadata);

  // 流信息
  for (unsigned int i = 0; i < format_context_ptr->nb_streams; i++) {
    const AVStream* st = format_context_ptr->streams[i];

    // 视频流
    if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
        video_stream.empty()) {
      ParseVideoStream(i, st, base_info, video_stream);
    }
    // 音频流
    else if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      ParseAudioStream(i, st, audio_stream);
    }
    // 字幕流
    else if (st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
      ParseSubtitleStream(i, st, subtitle_stream);
    }
    // 附件流 (目前只处理字体类型)
    else if (st->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT &&
             st->codecpar->codec_id == AV_CODEC_ID_TTF) {
      ParseAttachmentStream(i, st, attachment);
    }
  }

  // 章节信息
  if (format_context_ptr->nb_chapters != 0) {
    GetChapterInfo(format_context_ptr->nb_chapters,
                   format_context_ptr->chapters, chapter);
  }

  return true;
}

bool FFmpegHelper::GetVideoInfoFromStream(const AVStream* stream,
                                          VideoStreamPtr& video_stream) {
  if (!pfn_avcodec_alloc_context3_ || !pfn_avcodec_free_context_ ||
      !pfn_avcodec_parameters_to_context_ || !pfn_avcodec_find_decoder_ ||
      !pfn_av_get_profile_name_ || !pfn_av_reduce_ || !pfn_avcodec_get_name_) {
    return false;
  }

  std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>
      avcodec_ctx(pfn_avcodec_alloc_context3_(nullptr),
                  [this](AVCodecContext* avctx) {
                    if (avctx) {
                      pfn_avcodec_free_context_(&avctx);
                    }
                  });
  if (0 >
      pfn_avcodec_parameters_to_context_(avcodec_ctx.get(), stream->codecpar)) {
    return false;
  }
  video_stream->codec_id = avcodec_ctx->codec_id;

  video_stream->bitrate = avcodec_ctx->bit_rate;
  video_stream->avg_frame_bitrate = av_q2d(stream->avg_frame_rate);
  video_stream->real_frame_bitrate = av_q2d(stream->r_frame_rate);
  video_stream->level = avcodec_ctx->level;
  auto profile = avcodec_ctx->profile;

  auto codec_name = pfn_avcodec_get_name_(avcodec_ctx->codec_id);
  video_stream->codec_name = codec_name ? codec_name : "";
  const AVCodec* avcodec = pfn_avcodec_find_decoder_(avcodec_ctx->codec_id);

  if (avcodec) {
    if (avcodec_ctx) {
      video_stream->codec_long_name = avcodec->long_name;
    }

    if (profile > 0) {
      auto name = pfn_av_get_profile_name_(avcodec, profile);
      video_stream->profile = name ? name : "";
    }
  }

  int dst_num = 0, dst_den = 0;
  // max Maximum allowed values for `dst_num` & `dst_den`
  int64_t num_den_max = 1024 * 1024;
  pfn_av_reduce_(&dst_num, &dst_den,
                 avcodec_ctx->width * (int64_t)stream->sample_aspect_ratio.num,
                 avcodec_ctx->height * (int64_t)stream->sample_aspect_ratio.den,
                 num_den_max);
  base::SStringPrintf(&video_stream->aspect_ratio, "%d:%d", dst_num, dst_den);

  // HDR info
  // AVCOL_TRC_SMPTE2084：对应于PQ（Perceptual Quantizer，ST 2084）
  // AVCOL_TRC_ARIB_STD_B67：对应于HLG（Hybrid Log-Gamma）
  if (stream->codecpar->color_primaries == AVCOL_PRI_BT2020) {
    if (stream->codecpar->color_trc == AVCOL_TRC_SMPTE2084 ||
        stream->codecpar->color_trc == AVCOL_TRC_ARIB_STD_B67) {
      video_stream->hdr_info = std::make_shared<HDRInfo>();
      video_stream->hdr_info->color_primaries =
          stream->codecpar->color_primaries;
      video_stream->hdr_info->color_trc = stream->codecpar->color_trc;
      video_stream->hdr_info->color_space = stream->codecpar->color_space;
    }
  }

  GetMetadataFromAVDictionary(stream->metadata, video_stream->metadata);
  video_stream->ParseMetadata();

  return true;
}

bool FFmpegHelper::GetAudioInfoFromStream(const AVStream* stream,
                                          AudioStreamPtr& audio_stream) {
  if (!pfn_avcodec_find_decoder_ || !pfn_avcodec_get_name_) {
    return false;
  }

  audio_stream->codec_id = stream->codecpar->codec_id;
  audio_stream->channel = stream->codecpar->channels;
  audio_stream->sample_rate = stream->codecpar->sample_rate;
  audio_stream->bitrate = stream->codecpar->bit_rate;

  auto codec_name = pfn_avcodec_get_name_(stream->codecpar->codec_id);
  audio_stream->codec_name = codec_name ? codec_name : "";

  const AVCodec* avcodec =
      pfn_avcodec_find_decoder_(stream->codecpar->codec_id);

  if (avcodec) {
    audio_stream->codec_long_name = avcodec->long_name;
  }

  GetMetadataFromAVDictionary(stream->metadata, audio_stream->metadata);

  audio_stream->ParseMetadata();

  if (audio_stream->bitrate == 0) {
    audio_stream->bitrate = GetEstimatedAudioBitrate(stream->codecpar->codec_id,
                                                     audio_stream->channel);
  }
  return true;
}

bool FFmpegHelper::GetSubtitleInfoFromStream(
    const AVStream* stream,
    SubtitleStreamPtr& subtitle_stream) {
  if (!pfn_avcodec_find_decoder_ || !pfn_avcodec_get_name_) {
    return false;
  }
  subtitle_stream->codec_id = stream->codecpar->codec_id;
  auto codec_name = pfn_avcodec_get_name_(stream->codecpar->codec_id);
  subtitle_stream->codec_name = codec_name ? codec_name : "";

  const AVCodec* avcodec =
      pfn_avcodec_find_decoder_(stream->codecpar->codec_id);
  if (avcodec) {
    subtitle_stream->codec_long_name = avcodec->long_name;
  }

  GetMetadataFromAVDictionary(stream->metadata, subtitle_stream->metadata);
  subtitle_stream->ParseMetadata();
  return true;
}

bool FFmpegHelper::GetAttachmentInfoFromStream(
    const AVStream* stream,
    MediaAttachmentPtr& attachment_stream) {
  attachment_stream->codec_id = stream->codecpar->codec_id;
  attachment_stream->codec_name = "ttf";
  attachment_stream->codec_long_name = "TrueType font";
  GetMetadataFromAVDictionary(stream->metadata, attachment_stream->metadata);
  attachment_stream->ParseMetadata();

  return true;
}

bool FFmpegHelper::GetMetadataFromAVDictionary(const AVDictionary* avdictionart,
                                               MediaMetadata& metadata) {
  AVDictionaryEntry* tag = nullptr;
  while (
      (tag = pfn_av_dict_get_(avdictionart, "", tag, AV_DICT_IGNORE_SUFFIX))) {
    metadata.insert(std::pair<std::string, std::string>(tag->key, tag->value));
  }
  return true;
}

int64_t FFmpegHelper::GetEstimatedAudioBitrate(const AVCodecID& codec_id,
                                               int channels) {
  // 该函数主要是针对无法获取音频码率时，通过编码及音频通道数来预估码率值，该值参考
  // Jellyfin 项目中的经验值

  if (codec_id == AV_CODEC_ID_AAC || codec_id == AV_CODEC_ID_MP3) {
    if (channels <= 2) {
      return 192000;
    }
    if (channels >= 5) {
      return 320000;
    }
  } else if (codec_id == AV_CODEC_ID_AC3 || codec_id == AV_CODEC_ID_EAC3) {
    if (channels <= 2) {
      return 192000;
    }
    if (channels >= 5) {
      return 640000;
    }

  } else if (codec_id == AV_CODEC_ID_FLAC || codec_id == AV_CODEC_ID_ALAC) {
    if (channels <= 2) {
      return 960000;
    }
    if (channels >= 5) {
      return 2880000;
    }
  }
  return 0;
}

bool FFmpegHelper::SupportEncoder(std::string encoder_name) {
  if (!pfn_avcodec_find_encoder_by_name_ || !pfn_avcodec_alloc_context3_ ||
      !pfn_avcodec_free_context_ || !pfn_avcodec_open2_ ||
      !pfn_av_get_profile_name_ || !pfn_av_reduce_) {
    return false;
  }
  const AVCodec* avcodec =
      pfn_avcodec_find_encoder_by_name_(encoder_name.c_str());

  std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>>
      avcodec_ctx_output(pfn_avcodec_alloc_context3_(avcodec),
                         [this](AVCodecContext* avctx) {
                           if (avctx) {
                             pfn_avcodec_free_context_(&avctx);
                           }
                         });

  avcodec_ctx_output->width = 1920;
  avcodec_ctx_output->height = 1080;
  avcodec_ctx_output->time_base.num = 1;
  avcodec_ctx_output->time_base.den = 24;
  avcodec_ctx_output->pix_fmt = AV_PIX_FMT_NV12;
  avcodec_ctx_output->codec_id = avcodec->id;
  avcodec_ctx_output->codec_type = AVMEDIA_TYPE_VIDEO;

  int ret = pfn_avcodec_open2_(avcodec_ctx_output.get(), avcodec, nullptr);
  if (ret < 0) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
    pfn_av_strerror_(ret, errbuf, sizeof(errbuf));
    LOG(WARNING) << "Failed to open codec : " << encoder_name.c_str()
                 << " , error: " << ret << ", err_msg: " << errbuf;
    return false;
  } else {
    LOG(INFO) << "support encoder , " << encoder_name.c_str() << std::endl;
    return true;
  }
}
bool FFmpegHelper::IntelQsv() {
  DecodeContext decode = {nullptr};
  const AVCodec* encoder = nullptr;

  if (!pfn_av_hwdevice_ctx_create_ || !pfn_avcodec_find_encoder_by_name_) {
    return false;
  }

  bool ret = false;

  do {
    AVDictionary* opts = nullptr;
    auto r_value = pfn_av_hwdevice_ctx_create_(
        &decode.hw_device_ref, AV_HWDEVICE_TYPE_QSV, "auto_any", opts, 0);
    if (r_value < 0) {
      char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
      pfn_av_strerror_(r_value, errbuf, sizeof(errbuf));
      LOG(WARNING) << ("IntelQsv Cannot open the hardware device, error: ")
                   << errbuf;
      break;
    }

    if (!SupportEncoder("h264_qsv")) {
      LOG(WARNING) << "not support h264_qsv";
      break;
    }

    encoder = pfn_avcodec_find_encoder_by_name_("h264_qsv");
    if (!encoder) {
      LOG(INFO) << ("The QSV encoder is not present in libavcodec");
      break;
    }
    ret = true;
  } while (false);
  return ret;
}

bool FFmpegHelper::NVIDIACuda() {
  bool ret = false;
  if (!pfn_av_hwdevice_ctx_create_) {
    return false;
  }
  do {
    DecodeContext decode = {nullptr};
    auto t_ret = pfn_av_hwdevice_ctx_create_(
        &decode.hw_device_ref, AV_HWDEVICE_TYPE_CUDA, "auto", nullptr, 0);
    if (t_ret < 0) {
      char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
      pfn_av_strerror_(t_ret, err_buf, AV_ERROR_MAX_STRING_SIZE);
      LOG(ERROR) << ("CUDA Cannot open the hardware device, error: ") << t_ret
                 << ", err_str: " << err_buf;
      break;
    }

    if (!SupportEncoder("h264_nvenc")) {
      std::cout << "not support h264_nvenc";
      break;
    }
    ret = true;
  } while (false);
  return ret;
}

bool FFmpegHelper::GeneralDxva2() {
  DecodeContext decode = {nullptr};

  if (!pfn_av_hwdevice_ctx_create_) {
    return false;
  }
  auto t_ret = pfn_av_hwdevice_ctx_create_(
      &decode.hw_device_ref, AV_HWDEVICE_TYPE_DXVA2, "auto", nullptr, 0);
  if (t_ret < 0) {
    char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
    pfn_av_strerror_(t_ret, err_buf, AV_ERROR_MAX_STRING_SIZE);
    LOG(ERROR) << ("Dxva2 Cannot open the hardware device, error: ") << t_ret
               << ", err_str: " << err_buf;
    return false;
  }
  LOG(INFO) << "Dxva2 Succ";
  if (SupportEncoder("h264_nvenc")) {
    encoder_ = "h264_nvenc";
  } else if (SupportEncoder("h264_qsv")) {
    encoder_ = "h264_qsv";
  } else if (SupportEncoder("h264_amf")) {
    encoder_ = "h264_amf";
  } else {
    encoder_ = "libx264";
  }

  return t_ret >= 0;
}

bool FFmpegHelper::Vaapi() {
  bool ret = false;
  if (!pfn_av_hwdevice_ctx_create_) {
    return false;
  }
  do {
    DecodeContext decode = {nullptr};
    const char* device = "/dev/dri/renderD128";
    int t_ret = pfn_av_hwdevice_ctx_create_(
        &decode.hw_device_ref, AV_HWDEVICE_TYPE_VAAPI, device, NULL, 0);
    // auto t_ret = pfn_av_hwdevice_ctx_create_(
    //     &decode.hw_device_ref, AV_HWDEVICE_TYPE_VAAPI, "auto", nullptr, 0);
    if (t_ret < 0) {
      char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
      pfn_av_strerror_(t_ret, err_buf, AV_ERROR_MAX_STRING_SIZE);
      LOG(ERROR) << ("Vaapi Cannot open the hardware device, error: ") << t_ret
                 << ", err_str: " << err_buf;
      break;
    }
    // if (!SupportEncoder("h264_vaapi")) {
    //   std::cout << "not support h264_vaapi";
    //   break;
    // }
    encoder_ = "h264_vaapi";
    ret = true;
  } while (false);
  return ret;
}

base::NativeLibrary FFmpegHelper::GetDllModuleHandle(
    base::FilePath::StringPieceType dll_name) {
  auto dll_path = ffmpeg_dir_path_.Append(dll_name);
  base::NativeLibraryLoadError error;
  base::NativeLibrary lib_dll = base::LoadNativeLibrary(dll_path, &error);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary  failed, path: " << dll_path.value()
               << "error: " << error.ToString();
    return nullptr;
  }
  return lib_dll;
}

bool FFmpegHelper::InitFFmpegFuncFromAvcodec() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name =
      FILE_PATH_LITERAL("avcodec-59.dll");
#else
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("libavcodec.so");
#endif

  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitFFmpegFuncFromAvcodec failed\n";
    return false;
  }

  GetFunctionByDll(&pfn_avcodec_alloc_context3_, lib_dll,
                   "avcodec_alloc_context3");

  GetFunctionByDll(&pfn_avcodec_parameters_to_context_, lib_dll,
                   "avcodec_parameters_to_context");
  GetFunctionByDll(&pfn_avcodec_find_decoder_, lib_dll, "avcodec_find_decoder");
  GetFunctionByDll(&pfn_avcodec_find_encoder_, lib_dll, "avcodec_find_encoder");
  GetFunctionByDll(&pfn_avcodec_find_encoder_by_name_,

                   lib_dll, "avcodec_find_encoder_by_name");
  GetFunctionByDll(&pfn_avcodec_find_decoder_by_name_,

                   lib_dll, "avcodec_find_decoder_by_name");
  GetFunctionByDll(&pfn_avcodec_open2_, lib_dll, "avcodec_open2");
  GetFunctionByDll(&pfn_av_packet_alloc_, lib_dll, "av_packet_alloc");
  GetFunctionByDll(&pfn_av_new_packet_, lib_dll, "av_new_packet");
  GetFunctionByDll(&pfn_avcodec_send_packet_, lib_dll, "avcodec_send_packet");
  GetFunctionByDll(&pfn_avcodec_receive_frame_, lib_dll,
                   "avcodec_receive_frame");
  GetFunctionByDll(&pfn_avcodec_send_frame_, lib_dll, "avcodec_send_frame");
  GetFunctionByDll(&pfn_av_packet_free_, lib_dll, "av_packet_free");
  GetFunctionByDll(&pfn_avcodec_receive_packet_, lib_dll,
                   "avcodec_receive_packet");
  GetFunctionByDll(&pfn_av_packet_unref_, lib_dll, "av_packet_unref");

  GetFunctionByDll(&pfn_av_packet_clone_, lib_dll, "av_packet_clone");
  GetFunctionByDll(&pfn_avcodec_free_context_, lib_dll, "avcodec_free_context");

  GetFunctionByDll(&pfn_av_get_profile_name_, lib_dll, "av_get_profile_name");
  GetFunctionByDll(&pfn_avcodec_get_name_, lib_dll, "avcodec_get_name");

  return true;
}
bool FFmpegHelper::InitFFmpegFuncFromAvformat() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name =
      FILE_PATH_LITERAL("avformat-59.dll");
#else
  base::FilePath::StringPieceType dll_name =
      FILE_PATH_LITERAL("libavformat.so");
#endif
  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitFFmpegFuncFromAvformat failed";
    return false;
  }
  GetFunctionByDll(&pfn_avformat_open_input_, lib_dll, "avformat_open_input");
  GetFunctionByDll(&pfn_avformat_close_input_, lib_dll, "avformat_close_input");
  GetFunctionByDll(&pfn_av_seek_frame_, lib_dll, "av_seek_frame");
  GetFunctionByDll(&pfn_av_read_frame_, lib_dll, "av_read_frame");
  GetFunctionByDll(&pfn_av_dump_format_, lib_dll, "av_dump_format");
  GetFunctionByDll(&pfn_avformat_find_stream_info_, lib_dll,
                   "avformat_find_stream_info");

  GetFunctionByDll(&pfn_av_find_best_stream_, lib_dll, "av_find_best_stream");

  return true;
}
bool FFmpegHelper::InitFFmpegFuncFromAvutil() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("avutil-57.dll");
#else
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("libavutil.so");
#endif

  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitFFmpegFuncFromAvutil failed";
    return false;
  }
  GetFunctionByDll(&pfn_av_dict_get_, lib_dll, "av_dict_get");
  GetFunctionByDll(&pfn_av_frame_free_, lib_dll, "av_frame_free");
  GetFunctionByDll(&pfn_av_frame_clone_, lib_dll, "av_frame_clone");
  GetFunctionByDll(&pfn_av_frame_alloc_, lib_dll, "av_frame_alloc");
  GetFunctionByDll(&pfn_av_frame_get_buffer_, lib_dll, "av_frame_get_buffer");
  GetFunctionByDll(&pfn_av_hwdevice_ctx_create_, lib_dll,
                   "av_hwdevice_ctx_create");
  GetFunctionByDll(&pfn_av_rescale_q_, lib_dll, "av_rescale_q");
  GetFunctionByDll(&pfn_av_reduce_, lib_dll, "av_reduce");
  GetFunctionByDll(&pfn_av_log_set_level_, lib_dll, "av_log_set_level");
  GetFunctionByDll(&pfn_av_strerror_, lib_dll, "av_strerror");
  GetFunctionByDll(&pfn_av_display_rotation_get_, lib_dll,
                   "av_display_rotation_get");
  return true;
}
bool FFmpegHelper::InitFFmpegFuncFromSwscale() {
#if BUILDFLAG(IS_WIN)
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("swscale-6.dll");
#else
  base::FilePath::StringPieceType dll_name = FILE_PATH_LITERAL("libswscale.so");

#endif
  base::NativeLibrary lib_dll = GetDllModuleHandle(dll_name);
  if (lib_dll == nullptr) {
    LOG(ERROR) << "LoadNativeLibrary InitFFmpegFuncFromSwscale failed";
    return false;
  }
  GetFunctionByDll(&pfn_sws_get_context_, lib_dll, "sws_getContext");
  GetFunctionByDll(&pfn_sws_free_context_, lib_dll, "sws_freeContext");
  GetFunctionByDll(&pfn_sws_scale_, lib_dll, "sws_scale");
  return true;
}

template <class T>
void FFmpegHelper::GetFunctionByDll(T* func_ptr,
                                    base::NativeLibrary dll_handle,
                                    const char* func_name) {
  if (!func_ptr) {
    return;
  }
  *func_ptr =
      (T)base::GetFunctionPointerFromNativeLibrary(dll_handle, func_name);
}

bool FFmpegHelper::ParseVideoStream(int index,
                                    const AVStream* stream,
                                    MediaBasicInfoPtr& base_info,
                                    std::vector<VideoStreamPtr>& video_stream) {
  VideoStreamPtr single_video_stream = std::make_shared<VideoStream>();
  single_video_stream->stream_index = index;
  if (!GetVideoInfoFromStream(stream, single_video_stream)) {
    return false;
  }

  for (int i = 0; i < stream->nb_side_data; i++) {
    AVPacketSideData* side_data = &stream->side_data[i];
    if (side_data->type == AV_PKT_DATA_DISPLAYMATRIX) {
      int32_t matrix[9] = {0};
      memcpy(matrix, side_data->data, sizeof(matrix));
      double rotation_angle = pfn_av_display_rotation_get_(matrix);
      int angle = (int)rotation_angle % 360;
      single_video_stream->rotate = angle;
      break;
    }
  }

  base_info->width = stream->codecpar->width;
  base_info->height = stream->codecpar->height;
  video_stream.push_back(single_video_stream);
  return true;
}

bool FFmpegHelper::ParseAudioStream(int index,
                                    const AVStream* stream,
                                    std::vector<AudioStreamPtr>& audio_stream) {
  AudioStreamPtr single_audio_stream = std::make_shared<AudioStream>();
  single_audio_stream->stream_index = index;
  if (!GetAudioInfoFromStream(stream, single_audio_stream)) {
    return false;
  }

  audio_stream.push_back(single_audio_stream);
  return true;
}

bool FFmpegHelper::ParseSubtitleStream(
    int index,
    const AVStream* stream,
    std::vector<SubtitleStreamPtr>& subtitle_stream) {
  SubtitleStreamPtr single_subtitle_stream = std::make_shared<SubtitleStream>();

  single_subtitle_stream->stream_index = index;

  if (!GetSubtitleInfoFromStream(stream, single_subtitle_stream)) {
    return false;
  }
  subtitle_stream.push_back(single_subtitle_stream);
  return true;
}

bool FFmpegHelper::ParseAttachmentStream(
    int index,
    const AVStream* stream,
    std::vector<MediaAttachmentPtr>& attachment_stream) {
  MediaAttachmentPtr single_attachment_stream =
      std::make_shared<MediaAttachment>();
  single_attachment_stream->stream_index = index;

  if (!GetAttachmentInfoFromStream(stream, single_attachment_stream)) {
    return false;
  }
  attachment_stream.push_back(single_attachment_stream);
  return true;
}

void FFmpegHelper::GetChapterInfo(unsigned int nb_chapters,
                                  AVChapter** chapters,
                                  std::vector<MediaChapterPtr>& chapter) {
  for (unsigned int i = 0; i < nb_chapters; i++) {
    const AVChapter* st = chapters[i];

    MediaChapterPtr single_chapter = std::make_shared<MediaChapter>();

    single_chapter->stream_index = i;
    auto time_base = av_q2d(st->time_base);
    single_chapter->start_time_sec = st->start * time_base;
    single_chapter->end_time_sec = st->end * time_base;
    GetMetadataFromAVDictionary(st->metadata, single_chapter->metadata);
    for (const auto& val : single_chapter->metadata) {
      if (val.first.find("title") != -1) {
        single_chapter->title = val.second;
        break;
      }
    }
    chapter.push_back(single_chapter);
  }
}

}  // namespace nas
