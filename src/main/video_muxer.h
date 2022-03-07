#ifndef VXMT_VCAM_SHARE_VIDEO_MUXER
#define VXMT_VCAM_SHARE_VIDEO_MUXER

#include <iostream>

namespace vcamshare {
    class VideoMuxer {
        public:
        VideoMuxer(int w, int h, char *extraData, int extraLen, std::string filePath);
        ~VideoMuxer();
        void open();
        void close();
    };
}

#endif