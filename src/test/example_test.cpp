#define BOOST_TEST_MODULE My Example Test
#include <boost/test/included/unit_test.hpp>
#include "../main/video_muxer.h"

BOOST_AUTO_TEST_CASE(first_test)
{
  using namespace vcamshare;
  uint8_t spspps[] = {1, 2, 3};
  VideoMuxer muxer {1920, 1080, spspps, 3, 
          "/tmp/vcamsharetest.mp4"};

  uint8_t data[] = {0, 0, 0, 1, 2, 2, 2, 2, 2};
  muxer.writeVideoFrames(data, 9);
  muxer.writeAudioFrames(data, 9);

  muxer.close();
  std::cout << "muxer closed" << std::endl;
  // int i = 1;
  // BOOST_TEST(i);
  // BOOST_TEST(i == 2);
}