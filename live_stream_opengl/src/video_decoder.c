#include "video_decoder.h"

int decoder_init(decoder_t *dec, const int width, const int height)
{
	av_register_all();

	dec->pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (dec->pCodec == NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}

	dec->pCodecCtx = avcodec_alloc_context3(dec->pCodec);
	if (!dec->pCodecCtx)
	{
		fprintf(stderr, "Could not allocate video codec context\n");
		return 1;
	}

	dec->packet = av_packet_alloc();
	if (!dec->packet)
	{
		fprintf(stderr, "Could not alloc packet!\n");
		return 1;
	}

	dec->pCodecCtx->bit_rate = 2000000;
	dec->pCodecCtx->width = width;
	dec->pCodecCtx->height = height;
	dec->pCodecCtx->framerate = (AVRational){60, 1};

	if (avcodec_open2(dec->pCodecCtx, dec->pCodec, NULL) < 0)
	{
		printf("Could not open codec.\n");
		return -1;
	}
	dec->decoded_buffer = malloc(width * height * 3 / 2);
	dec->size = width * height * 3 / 2;
	return 0;
}
int decode(decoder_t *dec, uint8_t *buffer_to_decode, const int size)
{
	const int y_size = dec->pCodecCtx->width * dec->pCodecCtx->height;

	dec->packet->data = buffer_to_decode;
	dec->packet->size = size;

	int ret = avcodec_send_packet(dec->pCodecCtx, dec->packet);
	if (ret == 0)
	{
		AVFrame *pFrame = av_frame_alloc();
		ret = avcodec_receive_frame(dec->pCodecCtx, pFrame);
		if (ret == 0)
		{
			memcpy(dec->decoded_buffer, pFrame->data[0], y_size);
			memcpy(dec->decoded_buffer + y_size, pFrame->data[1], y_size / 4);
			memcpy(dec->decoded_buffer + y_size + y_size / 4, pFrame->data[2], y_size / 4);
			dec->decoder_flag = 1;
		}
		else
		{
			fprintf(stdout, "Decoding frame error\n");
			dec->decoder_flag = 0;
			return -1;
		}
		av_frame_free(&pFrame);
	}
	else
	{
		printf("Error sending packet to decoder!\n");
	}
	return y_size*3/2;
}
void decoder_destroy(decoder_t *dec)
{
	av_packet_free(&dec->packet);
	avcodec_free_context(&dec->pCodecCtx);
	av_packet_free(&dec->packet);
	free(dec->decoded_buffer);
	fprintf(stdout, "Decoder destroyed\n");
}

