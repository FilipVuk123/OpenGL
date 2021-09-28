#ifndef video_reader_h
#define video_reader_h

#define ORQA_IN
#define ORQA_REF

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct video_reader_t{
    int width; 
    int height;
    int video_stream_index; 

    AVFormatContext *av_format_ctx; 
    AVCodecContext *av_codec_ctx;
    AVFrame *av_frame; 
    AVPacket *av_packet;
    struct SwsContext *sws_scaler_ctx;
} video_reader_t;


int orqa_video_reader_open_file(ORQA_REF video_reader_t *state, ORQA_IN const char* filename);
uint8_t *orqa_video_reader_read_frame(ORQA_REF video_reader_t *state);
void orqa_video_reader_free(ORQA_REF video_reader_t *state);
const char* av_make_error(int errnum);
#endif