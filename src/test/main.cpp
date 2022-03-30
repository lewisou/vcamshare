#define BOOST_TEST_MODULE Test vCamShare Library
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/test/included/unit_test.hpp>
#include "../main/video_muxer.h"
#include "../main/utils.h"

BOOST_AUTO_TEST_SUITE(MuxerTest)

static void readH264File(std::string filePath, std::function<void(uint8_t*, int)> frameHandler) {
  std::ifstream source;
  source.open(filePath);
  if(source) {
    // std::cout << filePath << " opened" << std::endl;
    std::string line;

    while (std::getline(source, line)) {
      using namespace std;
      char space_char = ' ';
      vector<uint8_t> values {};

      stringstream sstream(line);
      string value;
      while (std::getline(sstream, value, space_char)){
          if(!value.empty()) {
            int v = stoi(value);
            if (v >= 0 && v <= 255) {
              values.push_back(v);
            } else {
              cerr << "value is out of range: " << v << endl;
            }
          }
          value.clear();
      }

      if(frameHandler) {
        frameHandler(values.data(), values.size());
      }
    }
  } else {
    std::cerr << "Failed to open test.h264" << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(muxing_searchSpsPps_return_ok)
{
  using namespace vcamshare;
  VideoMuxer muxer {1920, 1080, "/tmp/not_a_file"};
  uint8_t d1[] = {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 0, 0, 0, 1, 9};
  uint8_t *next = muxer.fillSpsPps(d1, boost::range_detail::array_size(d1));

  BOOST_TEST(next - d1 == boost::range_detail::array_size(d1) - 5);
  BOOST_TEST(next[4] == 9);

  uint8_t d2[] = {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 1, 2, 3, 1};
  uint8_t *next2 = muxer.fillSpsPps(d2, boost::range_detail::array_size(d2));

  BOOST_TEST(next2 == nullptr);

  uint8_t d3[] = {0, 0, 0, 1, 3, 6, 2, 2, 2, 2, 2, 1, 2, 3, 1, 5};
  uint8_t *next3 = muxer.fillSpsPps(d3, boost::range_detail::array_size(d3));

  BOOST_TEST(nullptr != next3);
  BOOST_TEST(d3 == next3);

  uint8_t d4[] = {0, 0, 0, 1, 7, 6, 2, 0, 0, 0, 1, 8, 2, 3, 1, 5, 0, 0, 0, 1, 6, 2, 3, 1, 5, 0, 0, 0, 1, 9, 2, 3, 1, 5};
  uint8_t *next4 = muxer.fillSpsPps(d4, boost::range_detail::array_size(d4));
  BOOST_TEST(next4[4] == 9);
}

BOOST_AUTO_TEST_CASE(muxing_searchSpsPps_spspps_filled)
{
  using namespace vcamshare;
  VideoMuxer muxer {0, 0, "/tmp/not_a_file"};
  uint8_t d1[] =    {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 0, 0, 0, 1, 9};
  uint8_t d1Exp[] = {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2};

  muxer.fillSpsPps(d1, boost::range_detail::array_size(d1));  
  auto spspps = muxer.getSpsPps();
  std::vector<uint8_t> expected(d1Exp, d1Exp + boost::range_detail::array_size(d1Exp));
  
  BOOST_TEST(expected == spspps);
}

BOOST_AUTO_TEST_CASE(muxing_searchSpsPps_spspps_filled_2)
{
  using namespace vcamshare;
  VideoMuxer muxer {0, 0, "/tmp/not_a_file"};
  uint8_t d1[] =    {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 0, 0, 0, 1, 8, 2, 2, 0, 0, 0, 1, 6, 2, 3, 0, 0, 0, 1, 9};
  uint8_t d1Exp[] = {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 0, 0, 0, 1, 8, 2, 2, 0, 0, 0, 1, 6, 2, 3};

  muxer.fillSpsPps(d1, boost::range_detail::array_size(d1));  
  auto spspps = muxer.getSpsPps();
  std::vector<uint8_t> expected(d1Exp, d1Exp + boost::range_detail::array_size(d1Exp));
  
  BOOST_TEST(expected == spspps);
}

BOOST_AUTO_TEST_CASE(muxing_searchSpsPps_spspps_filled_3)
{
  using namespace vcamshare;
  VideoMuxer muxer {0, 0, "/tmp/not_a_file"};
  uint8_t d1[] =    {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 0, 0, 0, 1, 8, 2, 2, 0, 2, 0, 1, 2};
  uint8_t d1Exp[] = {0, 0, 0, 1, 7, 6, 2, 2, 2, 2, 2, 0, 0, 0, 1, 8, 2, 2, 0, 2, 0, 1, 2};

  muxer.fillSpsPps(d1, boost::range_detail::array_size(d1));  
  auto spspps = muxer.getSpsPps();
  std::vector<uint8_t> expected(d1Exp, d1Exp + boost::range_detail::array_size(d1Exp));
  
  BOOST_TEST(expected == spspps);
}

BOOST_AUTO_TEST_CASE(mt_muxing_test)
{
  using namespace vcamshare;
  VideoMuxer muxer {1920, 1080, "/tmp/vcamsharetest2.mp4"};

  readH264File("mt.h264", [&muxer] (uint8_t *data, int len) {
    muxer.writeVideoFrames(data, len);
  });

  muxer.close();
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(UtilsTest)

BOOST_AUTO_TEST_CASE(searchH264Head)
{
    uint8_t data1[] = {0, 0, 0, 1, 2};
    uint8_t *rs = vcamshare::searchH264Head(data1, boost::range_detail::array_size(data1));
    BOOST_TEST(rs[4] == 2);

    uint8_t data2[] = {1, 4, 3, 0, 0, 0, 1, 2};
    uint8_t *rs2 = vcamshare::searchH264Head(data2, boost::range_detail::array_size(data2));
    BOOST_TEST(rs2 - data2 == 3);
}

BOOST_AUTO_TEST_CASE(searchH264Head_notFound)
{
    uint8_t data[] = {1, 4, 3, 0, 0, 1, 1, 2};
    uint8_t *rs = vcamshare::searchH264Head(data, boost::range_detail::array_size(data));
    BOOST_TEST(rs == nullptr);
}


BOOST_AUTO_TEST_SUITE_END()