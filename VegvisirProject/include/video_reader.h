#ifndef video_reader_h
#define video_reader_h

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS

typedef enum{
    VIDEO_FILE_OK           = 0,
    VIDEO_FILE_OPEN_ERROR   = -1,
    VIDEO_FILE_ALLOC_ERROR  = -2,
    VIDEO_FILE_INIT_ERROR   = -3,
    VIDEO_FILE_STREAM_ERROR = -4,
    VIDEO_FILE_ERROR        = -5
}VideoFileFlag;

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