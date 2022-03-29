#ifndef VXMT_VCAM_SHARE_VIDEO_MUXER
#define VXMT_VCAM_SHARE_VIDEO_MUXER

#include <iostream>

extern "C" {
#include <libavutil/timestamp.h>
#include <libavutil/mathematics.h>
#include <libavutil/error.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace vcamshare {

    typedef struct OutputStream {
        AVStream *st;
        AVCodecContext *enc;

        AVFrame *frame;
        AVFrame *tmp_frame;

        float t, tincr;
        int dts;
    } OutputStream;

    class VideoMuxer {
    public:
        VideoMuxer(int w, int h, uint8_t *extraData, int extraLen, 
                    std::string filePath);
        ~VideoMuxer();

        // expected data stream 00 00 00 01 xx xx xx xx
        void writeVideoFrames(const uint8_t *data, int len);

        // expected data stram ADTS
        void writeAudioFrames(const uint8_t *data, int len);

        void open();
        void close();

    private:
        void addFrames(const uint8_t *data, int len, bool video);
        void addStream(OutputStream *ost, AVFormatContext *oc,
                            const AVCodec **codec,
                            enum AVCodecID codec_id,
                            uint8_t *extra,
                            int extra_len);

        AVFormatContext *outputCtx;
        const AVCodec *videoCodec;
        const AVCodec *audioCodec;
        OutputStream videoSt;
        OutputStream audioSt;

        int mWidth, mHeight;
        std::string mFilePath;

        uint8_t mExtraData[256];
        int mExtraLen;
    };
}

#endif