#ifndef video_reader_h
#define video_reader_h

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <inttypes.h>

typedef struct video_reader{
    int width;
    int height;
    uint8_t *frame_buffer;
    int video_stream_index;

    AVFormatContext *av_format_ctx;
    AVCodecContext *av_codec_ctx;
    AVFrame *av_frame;
    AVPacket *av_packet;
} video_reader;

int video_reader_open_file(video_reader *state, const char *filename);

uint8_t video_reader_read_frame(video_reader *state);

void video_reader_free(video_reader *state);

#endif