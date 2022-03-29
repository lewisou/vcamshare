
#include "video_muxer.h"

extern "C" {
#define __STDC_CONSTANT_MACROS
#include <libavutil/timestamp.h>
#include <libavutil/mathematics.h>
#include <libavutil/error.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
}

#define STREAM_FRAME_RATE 30 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */


namespace vcamshare {

    VideoMuxer::VideoMuxer(int w, int h, uint8_t *extraData, int extraLen, std::string filePath) {
        mWidth = w;
        mHeight = h;
        mFilePath = filePath;
        mExtraLen = extraLen;
        memcpy(mExtraData, extraData, mExtraLen);

        open();
    }

    VideoMuxer::~VideoMuxer() {
        close();
    }

    void VideoMuxer::open() {
        int ret;

        std::cout << "video file: " << mFilePath << std::endl;

        videoSt.dts = 1;
        audioSt.dts = 1;

        avformat_alloc_output_context2(&outputCtx, NULL, NULL, mFilePath.c_str());
        if (!outputCtx) {
            std::cerr << "Failed to open file: " << mFilePath << std::endl;
            goto end;
        }
        
        addStream(&videoSt, outputCtx, &videoCodec, AV_CODEC_ID_H264, mExtraData, mExtraLen);
        addStream(&audioSt, outputCtx, &audioCodec, AV_CODEC_ID_AAC, NULL, 0);

        if (!(outputCtx->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&outputCtx->pb, mFilePath.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0) {
                std::cerr << "Could not open output context." << std::endl;
                goto end;
            }
        }

        av_dump_format(outputCtx, 0, mFilePath.c_str(), 1);
        ret = avformat_write_header(outputCtx, NULL);
        std::cout << "Header written: " << ret << std::endl;

        if (ret < 0) {
            char mess[256];
            av_strerror(ret, mess, 256);
            std::cerr << mess << std::endl;
            goto end;
        }

        return;

        end:
        /* close output */
        if (outputCtx && !(outputCtx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&outputCtx->pb);
        }

        avformat_free_context(outputCtx);
    }
    
    void VideoMuxer::close() {
        if(outputCtx) {
            std::cout << "About to write the trailer" << std::endl;
            av_write_trailer(outputCtx);
            std::cout << "trailer written!" << std::endl;

            /* Close each codec. */
            std::cout << "1" << std::endl;
            avcodec_free_context(&videoSt.enc);

            std::cout << "2" << std::endl;
            avcodec_free_context(&audioSt.enc);

            std::cout << "3" << std::endl;
            /* close output */
            if (outputCtx && !(outputCtx->oformat->flags & AVFMT_NOFILE)) {
                avio_closep(&outputCtx->pb);
            }

            std::cout << "4" << std::endl;
            avformat_free_context(outputCtx);
            outputCtx = nullptr;
            std::cout << "5" << std::endl;
        }
    }

    void VideoMuxer::writeVideoFrames(const uint8_t *data, int len) {
        addFrames(data, len, true);
    }

    void VideoMuxer::writeAudioFrames(const uint8_t *data, int len) {
        addFrames(data, len, false);
    }

    void VideoMuxer::addFrames(const uint8_t *data, int len, bool video) {
        uint8_t *avdata = static_cast<uint8_t *>(av_malloc(len + AV_INPUT_BUFFER_PADDING_SIZE));
        memset(avdata + len, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(avdata, data, len);
        
        AVPacket pkt = { 0 };
        int rs = av_packet_from_data(&pkt, (uint8_t *)avdata, len + AV_INPUT_BUFFER_PADDING_SIZE);
        if(rs == 0) {
            OutputStream *stream = video ? &videoSt : &audioSt;
            pkt.stream_index = stream->st->index;
            pkt.dts = pkt.pts = stream->dts;
            pkt.duration = 1;
            pkt.pos = -1;
            av_packet_rescale_ts(&pkt, stream->enc->time_base, stream->st->time_base);
            int ret = av_interleaved_write_frame(outputCtx, &pkt);
            stream->dts ++;

            std::cout << "av_interleaved_write_frame: " << ret << std::endl;
        } else {
            std::cerr << "Failed to create AVPacket" << std::endl;
        }

        av_packet_unref(&pkt);
        std::cout << "pkt freed" << std::endl;
    }

    void VideoMuxer::addStream(OutputStream *ost, 
                                AVFormatContext *oc,
                                const AVCodec **codec,
                                enum AVCodecID codec_id,
                                uint8_t *extra,
                                int extra_len) {
        AVCodecContext *c;
        int i;

        /* find the encoder */
        *codec = avcodec_find_decoder(codec_id);
        if (!(*codec)) {
            fprintf(stderr, "Could not find encoder for '%s'\n",
                    avcodec_get_name(codec_id));
            exit(1);
        }

        ost->st = avformat_new_stream(oc, *codec);
        if (!ost->st) {
            fprintf(stderr, "Could not allocate stream\n");
            exit(1);
        }
        ost->st->id = oc->nb_streams-1;
        c = avcodec_alloc_context3(*codec);

        if (!c) {
            fprintf(stderr, "Could not alloc an encoding context\n");
            exit(1);
        }
        ost->enc = c;

        switch ((*codec)->type) {
            case AVMEDIA_TYPE_AUDIO:
                std::cout << "add audio stream " << std::endl;
                c->sample_fmt  = (*codec)->sample_fmts ?
                                (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                c->bit_rate    = 128000;
                c->sample_rate = 48000;

                if ((*codec)->supported_samplerates) {
                    c->sample_rate = (*codec)->supported_samplerates[0];
                    for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                        if ((*codec)->supported_samplerates[i] == 48000) {
                            c->sample_rate = 48000;
                        }
                            
                    }
                }
                c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
                c->channel_layout = AV_CH_LAYOUT_STEREO;

                if ((*codec)->channel_layouts) {
                    c->channel_layout = (*codec)->channel_layouts[0];
                    for (i = 0; (*codec)->channel_layouts[i]; i++) {
                        if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                            c->channel_layout = AV_CH_LAYOUT_STEREO;
                    }
                }
                
                c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);

                ost->st->time_base = (AVRational){ 1, c->sample_rate / 1024 };
                c->time_base       = ost->st->time_base;
                break;

            case AVMEDIA_TYPE_VIDEO:
                std::cout << "add video stream " << std::endl;
                // avcodec_get_context_defaults3(c, *codec);
                c->codec_id = codec_id;

                c->bit_rate = 8000000;
                // c->sample_rate = 30;
                /* Resolution must be a multiple of two. */
                c->width    = mWidth;
                c->height   = mHeight;
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                * of which frame timestamps are represented. For fixed-fps content,
                * timebase should be 1/framerate and timestamp increments should be
                * identical to 1. */
                ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
                c->time_base       = ost->st->time_base;

                c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
                c->pix_fmt       = STREAM_PIX_FMT;
                c->profile = FF_PROFILE_H264_BASELINE;
                if(extra) {
                    uint8_t *avextra = (uint8_t *)av_mallocz(extra_len + AV_INPUT_BUFFER_PADDING_SIZE);
                    memset(avextra, 0, extra_len + AV_INPUT_BUFFER_PADDING_SIZE);
                    memcpy(avextra, extra, extra_len);
                    c->extradata = avextra;
                    c->extradata_size = extra_len;
                }

                if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                    /* just for testing, we also add B-frames */
                    c->max_b_frames = 2;
                }
                if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                    /* Needed to avoid using macroblocks in which some coeffs overflow.
                    * This does not happen with normal video, it just happens here as
                    * the motion of the chroma plane does not match the luma plane. */
                    c->mb_decision = 2;
                }
                c->time_base       = ost->st->time_base;
                break;

            default:
                break;
        }

        int ret = avcodec_parameters_from_context(ost->st->codecpar, c);
        if (ret < 0) {
           fprintf(stderr, "Could not copy the stream parameters\n");
            exit(1);
        }

        if((*codec)->type == AVMEDIA_TYPE_AUDIO) {
            ost->st->codecpar->channels = 2;
            ost->st->codecpar->sample_rate = 48000;
            ost->st->codecpar->frame_size = 1024;
        }

        /* Some formats want stream headers to be separate. */
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}