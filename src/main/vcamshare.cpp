#include "vcamshare.h"
#include "video_muxer.h"
#include <map>
#include <memory>
#include <iostream>

static int gHandler = 1;

static std::map<int, std::unique_ptr<vcamshare::VideoMuxer>> gVideoMuxers;

int createVideoMuxer(int w, int h, const char* filePath) {
    auto p = std::unique_ptr<vcamshare::VideoMuxer>(new vcamshare::VideoMuxer(w, h, filePath));

    gVideoMuxers[gHandler] = std::move(p);
    auto hd = gHandler;
    gHandler ++;
    return hd;
}

int closeVideoMuxer(int hd) {
    gVideoMuxers[hd] = nullptr;
}

void writeVideoFrames(int hd, uint8_t * const data, int len) {
    if(gVideoMuxers[hd]) {
        gVideoMuxers[hd]->writeVideoFrames(data, len);
        std::cout << "Frame written" << std::endl;
    } else {
        std::cerr << "muxer not found!" << std::endl;
    }
}

void writeAudioFrames(int hd, uint8_t * const data, int len) {
    if(gVideoMuxers[hd]) {
        gVideoMuxers[hd]->writeAudioFrames(data, len);
    }
}
