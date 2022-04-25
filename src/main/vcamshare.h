#ifndef VXMT_VCAM_SHARE_IOS
#define VXMT_VCAM_SHARE_IOS
#include "stdint.h"

extern "C"
int createVideoMuxer(int w, int h, const char* filePath);

extern "C"
int closeVideoMuxer(int hd);

extern "C"
void writeVideoFrames(int hd, uint8_t * const data, int len);

extern "C"
void writeAudioFrames(int hd, uint8_t * const data, int len);

#endif
