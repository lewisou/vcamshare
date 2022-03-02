#include "config.h"
#include "../main/muxer.h"

int main() {
    auto m = video::Muxer();
    m.hello();

    return CPPSHARE_VERSION_MAJOR;
}
