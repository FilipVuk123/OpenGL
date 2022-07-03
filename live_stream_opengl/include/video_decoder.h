#ifndef __VIDEO_DECODER_H__
#define __VIDEO_DECODER_H__

#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

typedef struct decoder_t
{
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVPacket *packet;

	int size;
	uint8_t *decoded_buffer;
	int decoder_flag;
} decoder_t;

void decoder_destroy(decoder_t *dec);

int decode(decoder_t *dec, uint8_t *buffer_to_decode, const int size);

int decoder_init(decoder_t *dec, const int width, const int height);

#endif 