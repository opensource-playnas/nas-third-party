
// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/02/14 15:28
#ifndef NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_HELPER_H_
#define NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_HELPER_H_

#include <map>
#include <string>
#include <vector>

#include "base/files/file.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/native_library.h"
#include "base/strings/utf_string_conversions.h"

#include "../media_info/media_struct.h"

#include "ffmpeg_api_define.h"
#include "ffmpeg_data_define.h"

namespace nas {

class FFmpegHelper {
 public:
  // Returns the singleton instance.
  static FFmpegHelper* GetInstance();
  FFmpegHelper(const FFmpegHelper&) = delete;
  FFmpegHelper& operator=(const FFmpegHelper&) = delete;

  void RunCommand();

 private:
  bool HandleInit();
  bool HandleParse(const base::FilePath& media_path);

  bool Init();

  // 在新增方法时，如果依赖ffmpeg 的函数指针,需要针对用到的函数指针做判空处理

  bool IsVideoFile(const base::FilePath file_path);

  bool ParseMediaInfo(MediaBasicInfoPtr& base_info,
                      std::vector<VideoStreamPtr>& video_stream,
                      std::vector<AudioStreamPtr>& audio_stream,
                      std::vector<SubtitleStreamPtr>& subtitle_stream,
                      std::vector<MediaChapterPtr>& chapter,
                      std::vector<MediaAttachmentPtr>& attachment);

  std::vector<HwAccels> GetHwAccels() { return hw_accels_; }
  std::string GetDefaultEncoder() { return encoder_; }

  base::FilePath GetToolsPath() { return ffmpeg_tools_path_; }

  // This object is a singleton:
  FFmpegHelper();
  ~FFmpegHelper();
  friend struct base::DefaultSingletonTraits<FFmpegHelper>;

  void InitHWAccels();

  template <class T>
  void GetFunctionByDll(T* func_ptr,
                        base::NativeLibrary dll_handle,
                        const char* func_name);

  bool ParseVideoStream(int index,
                        const AVStream* stream,
                        MediaBasicInfoPtr& base_info,
                        std::vector<VideoStreamPtr>& video_stream);

  bool ParseAudioStream(int index,
                        const AVStream* stream,
                        std::vector<AudioStreamPtr>& audio_stream);

  bool ParseSubtitleStream(int index,
                           const AVStream* stream,
                           std::vector<SubtitleStreamPtr>& subtitle_stream);

  bool ParseAttachmentStream(
      int index,
      const AVStream* stream,
      std::vector<MediaAttachmentPtr>& attachment_stream);

  void GetChapterInfo(unsigned int nb_chapters,
                      AVChapter** chapters,
                      std::vector<MediaChapterPtr>& chapter);

  bool GetVideoInfoFromStream(const AVStream* stream,
                              VideoStreamPtr& video_stream);
  bool GetAudioInfoFromStream(const AVStream* stream,
                              AudioStreamPtr& audio_stream);

  bool GetSubtitleInfoFromStream(const AVStream* stream,
                                 SubtitleStreamPtr& subtitle_stream);

  bool GetAttachmentInfoFromStream(const AVStream* stream,
                                   MediaAttachmentPtr& attachment_stream);

  bool GetMetadataFromAVDictionary(const AVDictionary* avdictionart,
                                   MediaMetadata& metadata);

  int64_t GetEstimatedAudioBitrate(const AVCodecID& codec_id, int channels);

  bool SupportEncoder(std::string encoder_name);
  bool IntelQsv();
  bool NVIDIACuda();
  bool GeneralDxva2();
  bool Vaapi();

  base::NativeLibrary GetDllModuleHandle(
      base::FilePath::StringPieceType dll_name);

  bool InitFFmpegFuncFromAvcodec();
  bool InitFFmpegFuncFromAvformat();
  bool InitFFmpegFuncFromAvutil();
  bool InitFFmpegFuncFromSwscale();

 private:
  bool init_succ_ = false;
  std::vector<HwAccels> hw_accels_;
  std::string encoder_;
  base::FilePath ffmpeg_dir_path_;
  base::FilePath ffmpeg_tools_path_;

  pfn_avformat_open_input pfn_avformat_open_input_ = nullptr;
  pfn_avformat_close_input pfn_avformat_close_input_ = nullptr;
  pfn_av_seek_frame pfn_av_seek_frame_ = nullptr;
  pfn_av_read_frame pfn_av_read_frame_ = nullptr;
  pfn_av_dump_format pfn_av_dump_format_ = nullptr;
  pfn_avformat_find_stream_info pfn_avformat_find_stream_info_ = nullptr;
  pfn_av_find_best_stream pfn_av_find_best_stream_ = nullptr;

  pfn_av_dict_get pfn_av_dict_get_ = nullptr;
  pfn_av_frame_free pfn_av_frame_free_ = nullptr;
  pfn_av_frame_clone pfn_av_frame_clone_ = nullptr;
  pfn_av_frame_alloc pfn_av_frame_alloc_ = nullptr;
  pfn_av_frame_get_buffer pfn_av_frame_get_buffer_ = nullptr;
  pfn_av_hwdevice_ctx_create pfn_av_hwdevice_ctx_create_ = nullptr;
  pfn_av_rescale_q pfn_av_rescale_q_ = nullptr;
  pfn_av_reduce pfn_av_reduce_ = nullptr;
  pfn_av_log_set_level pfn_av_log_set_level_ = nullptr;
  pfn_av_strerror pfn_av_strerror_ = nullptr;
  pfn_av_display_rotation_get pfn_av_display_rotation_get_ = nullptr;

  pfn_avcodec_alloc_context3 pfn_avcodec_alloc_context3_ = nullptr;
  pfn_avcodec_parameters_to_context pfn_avcodec_parameters_to_context_ =
      nullptr;
  pfn_avcodec_find_coder pfn_avcodec_find_decoder_ = nullptr;
  pfn_avcodec_find_coder pfn_avcodec_find_encoder_ = nullptr;
  pfn_avcodec_find_coder_by_name pfn_avcodec_find_encoder_by_name_ = nullptr;
  pfn_avcodec_find_coder_by_name pfn_avcodec_find_decoder_by_name_ = nullptr;

  pfn_avcodec_open2 pfn_avcodec_open2_ = nullptr;
  pfn_av_packet_alloc pfn_av_packet_alloc_ = nullptr;
  pfn_av_new_packet pfn_av_new_packet_ = nullptr;
  pfn_avcodec_send_packet pfn_avcodec_send_packet_ = nullptr;
  pfn_avcodec_receive_frame pfn_avcodec_receive_frame_ = nullptr;
  pfn_avcodec_send_frame pfn_avcodec_send_frame_ = nullptr;
  pfn_av_packet_free pfn_av_packet_free_ = nullptr;
  pfn_avcodec_receive_packet pfn_avcodec_receive_packet_ = nullptr;
  pfn_av_packet_unref pfn_av_packet_unref_ = nullptr;
  pfn_av_packet_clone pfn_av_packet_clone_ = nullptr;
  pfn_avcodec_free_context pfn_avcodec_free_context_ = nullptr;
  pfn_av_get_profile_name pfn_av_get_profile_name_ = nullptr;
  pfn_avcodec_get_name pfn_avcodec_get_name_ = nullptr;

  pfn_sws_get_context pfn_sws_get_context_ = nullptr;
  pfn_sws_free_context pfn_sws_free_context_ = nullptr;
  pfn_sws_scale pfn_sws_scale_ = nullptr;
};

}  // namespace nas

#endif  // NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_HELPER_H_