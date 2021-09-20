#ifndef video_reader_h
#define video_reader_h

#define ORQA_IN
#define ORQA_REF

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


typedef struct video_reader{
    int width;
    int height;
    int video_stream_index;

    AVFormatContext *av_format_ctx;
    AVCodecContext *av_codec_ctx;
    AVFrame *av_frame;
    AVFrame *av_frame_rgb;
    AVPacket *av_packet;
    struct SwsContex *sws_scaler_ctx;
} video_reader;

int ORQA_video_reader_open_file(ORQA_REF video_reader *state, ORQA_IN const char* filename);
uint8_t *ORQA_video_reader_read_frame(ORQA_REF video_reader* state);
void ORQA_video_reader_free(ORQA_REF video_reader *state);
const char* av_make_error(int errnum);
#endif