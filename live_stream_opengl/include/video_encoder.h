#ifndef __VIDEO_ENCODER_H__
#define __VIDEO_ENCODER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>


typedef struct encoder_t{
    AVCodec *codec;
    AVCodecContext *pCodecCtx;
    AVFrame *pFrame;

    int size;
	uint8_t *encoded_buffer;
	int encoder_flag;
} encoder_t;

int encode(encoder_t *enc, uint8_t *buffer_to_encode);
int encoder_init(encoder_t *enc, const int width, const int height);
void encoder_destroy(encoder_t *enc);

#endif