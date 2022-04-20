#include "vcamshare_ios.h"
#include "video_muxer.h"

int createVideoMuxer(int w, int h, const char* filePath) {
    auto vm = vcamshare::VideoMuxer(w, h, filePath);
    return -1;
}

int closeVideoMuxer(int hd) {

}

void writeVideoFrames(int hd, uint8_t * const data, int len) {

}

void writeAudioFrames(int hd, uint8_t * const data, int len) {

}
