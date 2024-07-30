// copyright 2023 The Master Lu PC-Group Authors. All rights reserved.
// author leixiaohang@ludashi.com
// date 2023/01/30 17:22

#ifndef NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_API_DEFINE_H_
#define NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_API_DEFINE_H_

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
#include "libavutil/log.h"
#include "libswscale/swscale.h"
}

#if BUILDFLAG(IS_WIN)
#define DECL_NAME __cdecl
#else
#define DECL_NAME __attribute__((__cdecl__))
#endif

// avformat  api
typedef int(DECL_NAME* pfn_avformat_open_input)(AVFormatContext** ps,
                                                const char* filename,
                                                const AVInputFormat* fmt,
                                                AVDictionary** options);
typedef void(DECL_NAME* pfn_avformat_close_input)(AVFormatContext** ps);
typedef int(DECL_NAME* pfn_av_seek_frame)(AVFormatContext* s,
                                          int stream_index,
                                          int64_t timestamp,
                                          int flags);
typedef int(DECL_NAME* pfn_av_read_frame)(AVFormatContext* s, AVPacket* pkt);
typedef void(DECL_NAME* pfn_av_dump_format)(AVFormatContext* ic,
                                            int index,
                                            const char* url,
                                            int is_output);
typedef int(DECL_NAME* pfn_avformat_find_stream_info)(AVFormatContext* ic,
                                                      AVDictionary** options);

typedef int(DECL_NAME* pfn_av_find_best_stream)(AVFormatContext* ic,
                                                enum AVMediaType type,
                                                int wanted_stream_nb,
                                                int related_stream,
                                                const AVCodec** decoder_ret,
                                                int flags);

// avutil  api
typedef AVDictionaryEntry*(DECL_NAME* pfn_av_dict_get)(
    const AVDictionary* m,
    const char* key,
    const AVDictionaryEntry* prev,
    int flags);
typedef void(DECL_NAME* pfn_av_frame_free)(AVFrame** frame);
typedef AVFrame*(DECL_NAME* pfn_av_frame_clone)(const AVFrame* src);
typedef AVFrame*(DECL_NAME* pfn_av_frame_alloc)(void);
typedef int(DECL_NAME* pfn_av_frame_get_buffer)(AVFrame* frame, int align);
typedef int(DECL_NAME* pfn_av_hwdevice_ctx_create)(AVBufferRef** device_ctx,
                                                   enum AVHWDeviceType type,
                                                   const char* device,
                                                   AVDictionary* opts,
                                                   int flags);

typedef int64_t(DECL_NAME* pfn_av_rescale_q)(int64_t a,
                                             AVRational bq,
                                             AVRational cq) av_const;
typedef int(DECL_NAME* pfn_av_reduce)(int* dst_num,
                                      int* dst_den,
                                      int64_t num,
                                      int64_t den,
                                      int64_t max);
typedef void(DECL_NAME* pfn_av_log_set_level)(int level);

typedef int(DECL_NAME* pfn_av_strerror)(int errnum,
                                        char* errbuf,
                                        size_t errbuf_size);

typedef double(DECL_NAME* pfn_av_display_rotation_get)(const int32_t matrix[9]);

// avcodec  api
typedef AVCodecContext*(DECL_NAME* pfn_avcodec_alloc_context3)(
    const AVCodec* codec);
typedef int(DECL_NAME* pfn_avcodec_parameters_to_context)(
    AVCodecContext* codec,
    const AVCodecParameters* par);
typedef AVCodec*(DECL_NAME* pfn_avcodec_find_coder)(enum AVCodecID id);
typedef const AVCodec*(DECL_NAME* pfn_avcodec_find_coder_by_name)(
    const char* name);

typedef int(DECL_NAME* pfn_avcodec_open2)(AVCodecContext* avctx,
                                          const AVCodec* codec,
                                          AVDictionary** options);

typedef AVPacket*(DECL_NAME* pfn_av_packet_alloc)(void);
typedef int(DECL_NAME* pfn_av_new_packet)(AVPacket* pkt, int size);

typedef int(DECL_NAME* pfn_avcodec_send_packet)(AVCodecContext* avctx,
                                                const AVPacket* avpkt);

typedef int(DECL_NAME* pfn_avcodec_receive_frame)(AVCodecContext* avctx,
                                                  AVFrame* frame);
typedef void(DECL_NAME* pfn_av_packet_free)(AVPacket** pkt);

typedef int(DECL_NAME* pfn_avcodec_receive_packet)(AVCodecContext* avctx,
                                                   AVPacket* avpkt);
typedef int(DECL_NAME* pfn_avcodec_send_frame)(AVCodecContext* avctx,
                                               const AVFrame* frame);
typedef void(DECL_NAME* pfn_av_packet_unref)(AVPacket* pkt);
typedef AVPacket*(DECL_NAME* pfn_av_packet_clone)(const AVPacket* src);
typedef void(DECL_NAME* pfn_avcodec_free_context)(AVCodecContext** avctx);
typedef const char*(DECL_NAME* pfn_av_get_profile_name)(const AVCodec* codec,
                                                        int profile);
typedef const char*(DECL_NAME* pfn_avcodec_get_name)(enum AVCodecID id);

// swscale  api
typedef struct SwsContext*(DECL_NAME* pfn_sws_get_context)(
    int srcW,
    int srcH,
    enum AVPixelFormat srcFormat,
    int dstW,
    int dstH,
    enum AVPixelFormat dstFormat,
    int flags,
    SwsFilter* srcFilter,
    SwsFilter* dstFilter,
    const double* param);

typedef void(DECL_NAME* pfn_sws_free_context)(struct SwsContext* swsContext);

typedef int(DECL_NAME* pfn_sws_scale)(struct SwsContext* c,
                                      const uint8_t* const srcSlice[],
                                      const int srcStride[],
                                      int srcSliceY,
                                      int srcSliceH,
                                      uint8_t* const dst[],
                                      const int dstStride[]);

#endif  // NAS_MEDIA_SERVER_SRC_FFMPEG_TOOLS_FFMPEG_API_DEFINE_H_