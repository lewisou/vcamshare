
#include "video_muxer.h"

extern "C" {
#include "libavformat/avformat.h"
}


namespace vcamshare {
    VideoMuxer::VideoMuxer(int w, int h, char *extraData, int extraLen, std::string filePath) {
        const AVOutputFormat *gf = av_guess_format("mp4", "stage.mp4", NULL);
    }

    VideoMuxer::~VideoMuxer() {

    }

    void VideoMuxer::open() {

    }
    
    void VideoMuxer::close() {

    }
}
