#ifndef VXMT_VCAM_SHARE_VIDEO_MUXER
#define VXMT_VCAM_SHARE_VIDEO_MUXER

#include <iostream>
#include <vector>

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
        VideoMuxer(int w, int h, std::string filePath);
        ~VideoMuxer();

        // expected data stream 00 00 00 01 xx xx xx xx
        void writeVideoFrames(uint8_t * const data, int len);

        // expected data stram ADTS
        void writeAudioFrames(uint8_t * const data, int len);

        void open(uint8_t *extraData, int extraLen);
        void close();
        bool isOpen();
        std::vector<uint8_t> getSpsPps();
        uint8_t *fillSpsPps(uint8_t * const data, int len);
    private:
        void addFrames(uint8_t * const data, int len, bool video);
        bool addStream(OutputStream *ost, AVFormatContext *oc,
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

        std::vector<uint8_t> mSpsPps;
    };
}

#endif