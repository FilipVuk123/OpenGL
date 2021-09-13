#include "../include/video_reader.h"

int video_reader_open_file(video_reader *state, const char *filename){
    AVFormatContext *av_format_ctx = state->av_format_ctx;
    AVCodecContext *av_codec_ctx = state->av_codec_ctx;
    int video_stream_index = state->video_stream_index;
    AVFrame *av_frame = state->av_frame;
    AVPacket *av_packet = state->av_packet;
    

    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx){
        fprintf(stderr, "In file: %s, line: %d Could not create AVFormatContex!\n", __FILE__, __LINE__);
        return 0;
    }

    if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) != 0){
        fprintf(stderr, "In file: %s, line: %d Could not open video file!!\n", __FILE__, __LINE__);
        return 0;
    }

    AVCodecParameters *av_codec_params;
    AVCodec *av_codec;
    video_stream_index = -1;

    for (unsigned int i = 0; i < av_format_ctx->nb_streams; i++){
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);

        if(!av_codec){
            continue;
        }
        if(av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return 0;
    }

    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if(!av_codec_ctx){
        fprintf(stderr, "In file: %s, line: %d Could not create CodecContex!\n", __FILE__, __LINE__);
        return 0;
    }

    if(avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0){
        fprintf(stderr, "In file: %s, line: %d Could not initialize AVCodecContex!\n", __FILE__, __LINE__);
        return 0;
    }
    if(avcodec_open2(av_codec_ctx, av_codec, NULL) < 0){
        fprintf(stderr, "In file: %s, line: %d Could not open codec!\n", __FILE__, __LINE__);
        return 0;
    }

    av_frame = av_frame_alloc();
    if(!av_frame){
        fprintf(stderr, "In file: %s, line: %d Could not allocate frame!\n", __FILE__, __LINE__);
        return 0;
    }
    av_packet = av_packet_alloc();
    if(!av_packet){
        fprintf(stderr, "In file: %s, line: %d Could not allocate packet!\n", __FILE__, __LINE__);
        return 0;
    }
    return 1;
}

    
uint8_t video_reader_read_frame(video_reader *state){

    AVFormatContext *av_format_ctx = state->av_format_ctx;
    AVCodecContext *av_codec_ctx = state->av_codec_ctx;
    int video_stream_index = state->video_stream_index;
    AVFrame *av_frame = state->av_frame;
    AVPacket *av_packet = state->av_packet;
    

    int response;
    while(av_read_frame(av_format_ctx, av_packet) >= 0){ // stupid naming...
        if(av_packet->stream_index != video_stream_index) continue;
        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0){
            fprintf(stderr, "In file: %s, line: %s Failed to decode packet: %d\n", av_err2str(response), __FILE__, __LINE__);
            return 0;
        }
        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            continue;
        } else if (response < 0){
            fprintf(stderr, "In file: %s, line: %s Failed to decode frame: %d\n", av_err2str(response), __FILE__, __LINE__);
            return 0;
        }
        av_packet_unref(av_packet);
        break;
    } 

    uint8_t *data = calloc(av_frame->width*av_frame->height*3, sizeof(unsigned char));
    if(!data) printf("Failed!");
    for (int i = 0; i < av_frame->width; i++){
        for (int j = 0; j < av_frame->height; j++){ 
            *(data + j*av_frame->width*3 + 3*i) = av_frame->data[0][j* av_frame->linesize[0] + i];
            *(data + j*av_frame->width*3 + 3*i + 1) = av_frame->data[0][j* av_frame->linesize[0] + i];
            *(data + j*av_frame->width*3 + 3*i + 2) = av_frame->data[0][j* av_frame->linesize[0] + i];
        }
    }
    return *data;
}

void video_reader_free(video_reader *state){
    // clean up
    avformat_close_input(&state->av_format_ctx);
    avformat_free_context(state->av_format_ctx);
    av_frame_free(&state->av_frame);
    av_packet_free(&state->av_packet);
    avcodec_free_context(&state->av_codec_ctx);
}