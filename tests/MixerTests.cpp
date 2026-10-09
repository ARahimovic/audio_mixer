#include <gtest/gtest.h>

#include "core/Mixer.hpp"
#include "core/VolumeFilter.hpp"
#include "core/VolumeTable.hpp"

namespace AudioMixer {
TEST(AudioMixerTest, SumFeeds) {
  Mixer mixer;
  // same sample rate for easier test
  mixer.addFeed(std::make_unique<Feed>(48000, 48000, 0));
  mixer.addFeed(std::make_unique<Feed>(48000, 48000, 0));
  mixer.feed(0).queue().push({0, {10, 20, 50, 60}});
  mixer.feed(1).queue().push({0, {30, 50, 90, 100}});

  std::vector<Sample> out{};
  // first 4 output samples
  mixer.render(0, 4, out);
  // should add both feeds together and we get 4 samples
  // volume by default is 100

  EXPECT_EQ(out, std::vector<Sample>({40, 70, 140, 160}));
}

TEST(AudioMixerTest, SumFeedsWithVolume) {
  // 2 feeds
  VolumeTable volumes(2);
  volumes.setVolume(0, 50);
  volumes.setVolume(1, 10);

  Mixer mixer;
  mixer.addFeed(std::make_unique<Feed>(48000, 48000, 0));
  mixer.addFeed(std::make_unique<Feed>(48000, 48000, 0));
  mixer.feed(0).filters().addFilter(std::make_unique<VolumeFilter>(volumes, 0));
  mixer.feed(1).filters().addFilter(std::make_unique<VolumeFilter>(volumes, 1));
  mixer.feed(0).queue().push({0, {10, 20, 50, 60}});
  mixer.feed(1).queue().push({0, {30, 50, 90, 100}});

  std::vector<Sample> out{};
  mixer.render(0, 4, out);

  EXPECT_EQ(out, std::vector<Sample>({8, 15, 34, 40}));
}

TEST(AudioMixerTest, FeedWithUpsampling) {
  Mixer mixer;
  mixer.addFeed(std::make_unique<Feed>(8000, 16000, 0));

  mixer.feed(0).queue().push({0, {10, 20, 50, 60, 80}});

  std::vector<Sample> out{};
  mixer.render(0, 10, out);
  // last sample is interpolated from the last feed sample and zero
  EXPECT_EQ(out, std::vector<Sample>({10, 15, 20, 35, 50, 55, 60, 70, 80, 40}));
}

TEST(AudioMixerTest, FeedWithDownsampling) {
  Mixer mixer;

  mixer.addFeed(std::make_unique<Feed>(96000, 48000, 0));

  mixer.feed(0).queue().push(
      {0, {10, 20, 50, 60, 80, 100, 120, 140, 160, 180}});

  std::vector<Sample> out{};
  mixer.render(0, 5, out);
  // ratio is 2:1, so we get 5 samples from 10 input samples
  EXPECT_EQ(out, std::vector<Sample>({10, 50, 80, 120, 160}));
}

TEST(AudioMixerTest, SumFeedsUpsampledWithVolume) {
  VolumeTable volumes(2);
  volumes.setVolume(0, 50);
  volumes.setVolume(1, 10);

  Mixer mixer;
  mixer.addFeed(std::make_unique<Feed>(8000, 16000, 0));
  mixer.addFeed(std::make_unique<Feed>(8000, 16000, 0));
  mixer.feed(0).filters().addFilter(std::make_unique<VolumeFilter>(volumes, 0));
  mixer.feed(1).filters().addFilter(std::make_unique<VolumeFilter>(volumes, 1));
  mixer.feed(0).queue().push({0, {10, 20, 50, 60, 80}});
  mixer.feed(1).queue().push({0, {30, 50, 90, 100, 120}});

  std::vector<Sample> out{};
  mixer.render(0, 8, out);

  // feed 0 at 50%: 5, 7, 10, 17, 25, 27, 30, 35
  // feed 1 at 10%: 3, 4, 5, 7, 9, 9, 10, 11
  EXPECT_EQ(out, std::vector<Sample>({8, 11, 15, 24, 34, 36, 40, 46}));
}
}  // namespace AudioMixer