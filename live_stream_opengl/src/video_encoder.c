#include "video_encoder.h"

int encode(encoder_t *enc, uint8_t *buffer_to_encode)
{
    int ret;

    ret = av_frame_make_writable(enc->pFrame);
        if (ret < 0)
            return -1;

    memcpy(enc->pFrame->data[0], buffer_to_encode, enc->pCodecCtx->width * enc->pCodecCtx->height);
    memcpy(enc->pFrame->data[1], buffer_to_encode + enc->pCodecCtx->width * enc->pCodecCtx->height, enc->pCodecCtx->width * enc->pCodecCtx->height / 4);
    memcpy(enc->pFrame->data[2], buffer_to_encode + enc->pCodecCtx->width * enc->pCodecCtx->height + enc->pCodecCtx->width * enc->pCodecCtx->height / 4, enc->pCodecCtx->width * enc->pCodecCtx->height / 4);

    ret = avcodec_send_frame(enc->pCodecCtx, enc->pFrame);
    if (ret == 0)
    {
        AVPacket *packet = av_packet_alloc();
        ret = avcodec_receive_packet(enc->pCodecCtx, packet);
        if(ret == 0){
            enc->size = packet->size;
            memcpy(enc->encoded_buffer, packet->data, enc->size);
            enc->encoder_flag = 1;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error during encoding\n");
        }
        av_packet_free(&packet);
    } else {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return -1;
    }

    return enc->size;
}

int encoder_init(encoder_t *enc, const int width, const int height){
    int ret;
    if (width % 2 != 0 || height % 2 != 0)
    {
        fprintf(stderr, "resolution must be a multiple of two\n");
        return 1;
    }
    avcodec_register_all();

    enc->codec = avcodec_find_encoder(AV_CODEC_ID_H264); // soft encoding
    if (!enc->codec)
    {
        fprintf(stderr, "Codec not found\n");
        return 1;
    }

    enc->pCodecCtx = avcodec_alloc_context3(enc->codec);
    if (!enc->pCodecCtx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return 1;
    }


    enc->pCodecCtx->bit_rate = 2000000;
    enc->pCodecCtx->width = width;
    enc->pCodecCtx->height = height;
    enc->pCodecCtx->time_base = (AVRational){1, 60};
    enc->pCodecCtx->framerate = (AVRational){60, 1};
    enc->pCodecCtx->gop_size = 20;
    enc->pCodecCtx->max_b_frames = 0;
    enc->pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (enc->codec->id == AV_CODEC_ID_H264){
        ret = av_opt_set(enc->pCodecCtx->priv_data, "tune", "zerolatency", 0);
        if (ret != 0) printf("Could not set av_opt_set()\n");
    }
    av_log_set_level(AV_LOG_FATAL);

    /* open it */
    ret = avcodec_open2(enc->pCodecCtx, enc->codec, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open codec, ret = %d\n", ret);
        return 1;
    }

    enc->pFrame = av_frame_alloc();
    if (!enc->pFrame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        return 1;
    }
    enc->pFrame->format = enc->pCodecCtx->pix_fmt;
    enc->pFrame->width = enc->pCodecCtx->width;
    enc->pFrame->height = enc->pCodecCtx->height;

    ret = av_frame_get_buffer(enc->pFrame, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not allocate the video frame data\n");
        return 1;
    }
    enc->encoded_buffer = malloc(width*height*3/2);
    enc->encoder_flag = 0;
    enc->size = 0;
    return 0;
}

void encoder_destroy(encoder_t *enc){
    avcodec_free_context(&enc->pCodecCtx);
    av_frame_free(&enc->pFrame);
    free(enc->encoded_buffer);
    fprintf(stdout, "Encoder destroyed\n");
}

