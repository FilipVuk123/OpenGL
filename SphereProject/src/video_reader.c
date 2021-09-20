#include "video_reader.h"

const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

int ORQA_video_reader_open_file(video_reader *state, const char *filename){
    // Open the file using libavformat
    state->av_format_ctx = avformat_alloc_context();
    if (!state->av_format_ctx){
        fprintf(stderr, "In file: %s, line: %d Could not create AVFormatContex!\n", __FILE__, __LINE__);
        return 0;
    }

    if (avformat_open_input(&state->av_format_ctx, filename, NULL, NULL) != 0){
        fprintf(stderr, "In file: %s, line: %d Could not open video file!!\n", __FILE__, __LINE__);
        return 0;
    }
    
    // Find the valid video stream inside the file
    AVCodecParameters *av_codec_params;
    AVCodec *av_codec;
    state->video_stream_index = -1;

    for (unsigned int i = 0; i < state->av_format_ctx->nb_streams; i++){
        av_codec_params = state->av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);

        if(!av_codec){
            continue;
        }
        if(av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            state->video_stream_index = i;
            state->width = av_codec_params->width;
            state->height = av_codec_params->height;
            break;
        }
    }
    if (state->video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return 0;
    }

    // Set up a codec context for the decoder
    state->av_codec_ctx = avcodec_alloc_context3(av_codec);
    if(!state->av_codec_ctx){
        fprintf(stderr, "In file: %s, line: %d Could not create CodecContex!\n", __FILE__, __LINE__);
        return 0;
    }

    if(avcodec_parameters_to_context(state->av_codec_ctx, av_codec_params) < 0){
        fprintf(stderr, "In file: %s, line: %d Could not initialize AVCodecContex!\n", __FILE__, __LINE__);
        return 0;
    }
    if(avcodec_open2(state->av_codec_ctx, av_codec, NULL) < 0){
        fprintf(stderr, "In file: %s, line: %d Could not open codec!\n", __FILE__, __LINE__);
        return 0;
    }

    state->av_frame = av_frame_alloc();
    if(!state->av_frame){
        fprintf(stderr, "In file: %s, line: %d Could not allocate frame!\n", __FILE__, __LINE__);
        return 0;
    }

    state->av_packet = av_packet_alloc();
    if(!state->av_packet){
        fprintf(stderr, "In file: %s, line: %d Could not allocate packet!\n", __FILE__, __LINE__);
        return 0;
    }

    return 1;
}
   
uint8_t *ORQA_video_reader_read_frame(video_reader *state){
    int response;
    // Decode one frame
    while(av_read_frame(state->av_format_ctx, state->av_packet) == 0){
        if(state->av_packet->stream_index != state->video_stream_index) continue;
        response = avcodec_send_packet(state->av_codec_ctx, state->av_packet);
        if (response < 0){
            fprintf(stderr, "In file: %s, line: %s Failed to decode packet: %d\n", av_make_error(response), __FILE__, __LINE__);
            return 0;
        }
        response = avcodec_receive_frame(state->av_codec_ctx, state->av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            continue;
        } else if (response < 0){
            fprintf(stderr, "In file: %s, line: %s Failed to decode frame: %d\n", av_make_error(response), __FILE__, __LINE__);
            return 0;
        }
        av_packet_unref(state->av_packet);
        break;
    }  
    // load frame into data
    uint8_t *data = calloc(state->av_frame->width*state->av_frame->height*3, sizeof(unsigned char));
    if(!data) printf("Failed to allocate data!");
    for (int i = 0; i < state->av_frame->width; i++){
        for (int j = 0; j < state->av_frame->height; j++){ 
            *(data + j*state->av_frame->width*3 + 3*i) = state->av_frame->data[0][j* state->av_frame->linesize[0] + i];
            *(data + j*state->av_frame->width*3 + 3*i + 1) = state->av_frame->data[0][j* state->av_frame->linesize[0] + i];
            *(data + j*state->av_frame->width*3 + 3*i + 2) = state->av_frame->data[0][j* state->av_frame->linesize[0] + i];
        }
    }
    return data;
    /*
    if (!state->sws_scaler_ctx) {
        state->sws_scaler_ctx = sws_getContext(state->width, state->height, state->av_codec_ctx->pix_fmt,
                                        state->width, state->height, AV_PIX_FMT_RGB0,
                                        SWS_BILINEAR, NULL, NULL, NULL);
    }
    if (!state->sws_scaler_ctx) {
        printf("Couldn't initialize sw scaler\n");
        return 0;
    }
    uint8_t *frame_buffer = calloc(state->av_frame->width*state->av_frame->height*4, sizeof(unsigned char));
    uint8_t* dest[4] = { frame_buffer, NULL, NULL, NULL };
    free(frame_buffer);
    int dest_linesize[4] = { state->width * 4, 0, 0, 0 };
    // Assertion desc failed at src/libswscale/swscale_internal.h
    sws_scale(state->sws_scaler_ctx, state->av_frame->data, state->av_frame->linesize, 0, state->av_frame->height, dest, dest_linesize);
    return dest;*/
}

void ORQA_video_reader_free(video_reader *state){
    // clean up
    avformat_close_input(&state->av_format_ctx);
    avformat_free_context(state->av_format_ctx);
    av_frame_free(&state->av_frame);
    av_packet_free(&state->av_packet);
    avcodec_free_context(&state->av_codec_ctx);
    // sws_freeContext(state->sws_scaler_ctx);
}