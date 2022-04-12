
#include "video_muxer.h"
#include "utils.h"

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

    VideoMuxer::VideoMuxer(int w, int h, std::string filePath) {
        mWidth = w;
        mHeight = h;
        mFilePath = filePath;
        outputCtx = nullptr;
        videoSt.enc = nullptr;
        audioSt.enc = nullptr;
    }

    VideoMuxer::~VideoMuxer() {
        close();
    }

    void VideoMuxer::open(uint8_t *extraData, int extraLen) {
        if(outputCtx) return;

        int ret;

        std::cout << "video file: " << mFilePath << std::endl;

        videoSt.dts = 1;
        audioSt.dts = 1;

        avformat_alloc_output_context2(&outputCtx, NULL, NULL, mFilePath.c_str());
        if (!outputCtx) {
            std::cerr << "Failed to open file: " << mFilePath << std::endl;
            goto end;
        }
        
        if(!addStream(&videoSt, outputCtx, &videoCodec, AV_CODEC_ID_H264, extraData, extraLen)) {
            goto end;
        }
        if(!addStream(&audioSt, outputCtx, &audioCodec, AV_CODEC_ID_AAC, nullptr, 0)) {
            goto end;
        }

        av_dump_format(outputCtx, 0, mFilePath.c_str(), 1);
        
        if (!(outputCtx->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_open(&outputCtx->pb, mFilePath.c_str(), AVIO_FLAG_WRITE);
            if (ret < 0) {
                std::cerr << "Could not open output context." << std::endl;
                goto end;
            }
        }

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
        std::cerr << "Something wrong in the end section." << std::endl;
        close();
    }
    
    bool VideoMuxer::isOpen() {
        return outputCtx != nullptr;
    }

    void VideoMuxer::close() {
        if(outputCtx) {
            int rs = av_write_trailer(outputCtx);
            std::cout << "trailer written: " << rs << std::endl;
        }

        /* Close each codec. */
        if(videoSt.enc) {
            avcodec_free_context(&videoSt.enc);
            videoSt.enc = nullptr;
        }
        
        if(audioSt.enc) {
            avcodec_free_context(&audioSt.enc);
            audioSt.enc = nullptr;
        }

        /* close output */
        if (outputCtx && !(outputCtx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&outputCtx->pb);
        }

        if(outputCtx) {
            avformat_free_context(outputCtx);
            outputCtx = nullptr;
        }
    }

    std::vector<uint8_t> VideoMuxer::getSpsPps() {
        return mSpsPps;
    }

    uint8_t *VideoMuxer::fillSpsPps(uint8_t * const data, int len) {
        if ((data[4] & 0x1f) != 7) {
            return data;
        }

        int max = len;
        uint8_t *start = data;
        uint8_t *head = nullptr;
        int nalType = -1;
        do {
            head = searchH264Head(start + 1, max - 1);
            if (head) {
                nalType = head[4] & 0x1f;

                max -= (head - start);
                start = head;
            } else {
                nalType = -1;
                break;
            }
        } while (nalType == 7 || nalType == 8 || nalType == 6);

        // Now the head points to the next I/P nal or null.
        int orgNalType = data[4] & 0x1f;
        if (orgNalType == 7) {
            int spsPpsLen = head == nullptr ? len : (head - data);
            mSpsPps.clear();
            for (int i = 0; i < spsPpsLen; i ++) {
                mSpsPps.push_back(data[i]);
            }
        }
        return head;
    }

    void VideoMuxer::writeVideoFrames(uint8_t * const data, int len) {
        uint8_t *frame = fillSpsPps(data, len);

        if(!isOpen()) {
            if (mSpsPps.empty()) {
                return;
            }
            open(mSpsPps.data(), mSpsPps.size());
        }

        if(isOpen() && frame) {
            // int nalType = frame[4] & 0x1f;
            // if (nalType != 1) std::cout << "nalType: " << nalType << std::endl;
            // std::cout << "nalType: " << nalType << std::endl;
            
            // if (nalType == 5) {
            //     addFrames(mSpsPps.data(), mSpsPps.size(), true);    
            // }
            addFrames(frame, len - (frame - data), true);
            // addFrames(data, len, true);
        }
    }

    void VideoMuxer::writeAudioFrames(uint8_t * const data, int len) {
        addFrames(data, len, false);
    }

    void VideoMuxer::addFrames(uint8_t * const data, int len, bool video) {
        uint8_t *avdata = static_cast<uint8_t *>(av_malloc(len + AV_INPUT_BUFFER_PADDING_SIZE));
        memset(avdata + len, 0, AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(avdata, data, len);
        
        // int nalType = data[4] & 0x1f;
        // if (nalType != 1) std::cout << "nalType: " << nalType << std::endl;

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

            // std::cout << "av_interleaved_write_frame: " << ret << std::endl;
        } else {
            std::cerr << "Failed to create AVPacket" << std::endl;
        }

        av_packet_unref(&pkt);
        // std::cout << "pkt freed" << std::endl;
    }

    bool VideoMuxer::addStream(OutputStream *ost, 
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
            return false;
        }

        ost->st = avformat_new_stream(oc, *codec);
        if (!ost->st) {
            fprintf(stderr, "Could not allocate stream\n");
            return false;
        }
        ost->st->id = oc->nb_streams-1;
        c = avcodec_alloc_context3(*codec);

        if (!c) {
            fprintf(stderr, "Could not alloc an encoding context\n");
            return false;
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
                // if(extra) {
                //     uint8_t *avextra = (uint8_t *)av_mallocz(extra_len + AV_INPUT_BUFFER_PADDING_SIZE);
                //     memset(avextra, 0, extra_len + AV_INPUT_BUFFER_PADDING_SIZE);
                //     memcpy(avextra, extra, extra_len);
                //     c->extradata = avextra;
                //     c->extradata_size = extra_len;
                // }

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

        /* Some formats want stream headers to be separate. */
        if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        int ret = avcodec_parameters_from_context(ost->st->codecpar, c);
        if (ret < 0) {
            fprintf(stderr, "Could not copy the stream parameters\n");
            return false;
        }

        switch ((*codec)->type) {
            case AVMEDIA_TYPE_AUDIO:
                ost->st->codecpar->channels = 2;
                ost->st->codecpar->sample_rate = 48000;
                ost->st->codecpar->frame_size = 1024;
                break;
            case AVMEDIA_TYPE_VIDEO:
                if(extra) {
                    uint8_t *avextra = (uint8_t *)av_mallocz(extra_len + AV_INPUT_BUFFER_PADDING_SIZE);
                    memset(avextra, 0, extra_len + AV_INPUT_BUFFER_PADDING_SIZE);
                    memcpy(avextra, extra, extra_len);

                    ost->st->codecpar->extradata = avextra;
                    ost->st->codecpar->extradata_size = extra_len;
                }
                break;
        }

        return true;
    }
}