#include <gtest/gtest.h>

#include "core/Time.hpp"

namespace AudioMixer {
namespace {

TEST(TimeTest, ConvertsSampleIndexToTime) {
  EXPECT_EQ(timeOfSampleAtIndex(0, 441, 44100), 10000);
}

TEST(TimeTest, ConvertsTimeToSampleIndex) {
  EXPECT_EQ(indexOfSampleAt(0, 10000, 44100), 441);
}

TEST(TimeTest, RoundsToNearestSample) {
  // round up to the nearest index  which is 1
  EXPECT_EQ(indexOfSampleAt(0, 22, 44100), 1);
}

TEST(TimeTest, TimeLineTest) {
  Timeline timeline;
  timeline.streamStart = 1000000;
  timeline.wallStart = Clock::now();

  EXPECT_EQ(timeline.arrivalTime(1000050),
            timeline.wallStart + std::chrono::microseconds(50));
  EXPECT_EQ(timeline.playTime(1010000),
            timeline.wallStart + std::chrono::milliseconds(30));
}

}  // namespace
}  // namespace AudioMixer
