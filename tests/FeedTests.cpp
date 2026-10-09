#include <gtest/gtest.h>

#include <vector>

#include "core/Feed.hpp"
#include "core/Time.hpp"

namespace AudioMixer {
namespace {

TEST(FeedTest, TwoBlocksFeedWithGaps) {
  Timeline timeline;
  timeline.streamStart = 0;

  Feed feed(400, 400, 0);
  // first block will get index 0-3
  feed.queue().push({0, {10, 20, 30, 40}});
  // 20 ms later, at 400 Hz: 0.020 * 400 = sample 8, so this block is indexes
  // 8-11
  feed.queue().push({20000, {50, 60, 70, 80}});

  std::vector<Sample> out;

  feed.render(0, 4, out);
  EXPECT_EQ(out, std::vector<Sample>({10, 20, 30, 40}));

  feed.render(4, 4, out);
  // 4 samples of silence
  EXPECT_EQ(out, std::vector<Sample>({0, 0, 0, 0}));
  EXPECT_FALSE(feed.finished());

  feed.render(8, 4, out);
  EXPECT_EQ(out, std::vector<Sample>({50, 60, 70, 80}));

  // we now push the late block, which should be ignored
  feed.queue().push({10000, {90, 100, 110, 120}});
  feed.render(12, 4, out);
  EXPECT_EQ(out, std::vector<Sample>({0, 0, 0, 0}));
  EXPECT_EQ(feed.droppedSamples(), 4);

  feed.queue().close();
  EXPECT_TRUE(feed.finished());
}

TEST(FeedTest, DropsLateSamples) {
  Timeline timeline;
  timeline.streamStart = 0;

  Feed feed(400, 400, 0);
  std::vector<Sample> out;
  feed.render(0, 4, out);

  feed.queue().push({0, {10, 20, 30, 40}});
  // output index is 4, so those pushed samples are late
  feed.render(4, 4, out);

  EXPECT_EQ(out, std::vector<Sample>({0, 0, 0, 0}));
  EXPECT_EQ(feed.droppedSamples(), 4);
}

TEST(FeedTest, InterpolatesToHigherRate) {
  Feed feed(800, 1600, 0);
  feed.queue().push({0, {0, 100, 200}});

  std::vector<Sample> out;
  feed.render(0, 4, out);

  EXPECT_EQ(out, std::vector<Sample>({0, 50, 100, 150}));
}

}  // namespace
}  // namespace AudioMixer
