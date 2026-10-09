#include <gtest/gtest.h>

#include "core/Feed.hpp"
#include "core/Time.hpp"

#include <vector>

namespace AudioMixer
{
namespace
{

// at 400 Hz, one 10 ms period is 4 samples
TEST(FeedTest, PlaysBlocksAndGapsAsSilence)
{
    Timeline timeline;
    timeline.streamStart = 0;

    Feed feed(400, 400, 0);
    //first block will get index 0-3
    feed.queue().push({0, {10, 20, 30, 40}});
    //the second block will get indexs 8-11
    feed.queue().push({20000, {50, 60, 70, 80}});
    feed.queue().close();

    std::vector<Sample> out;

    feed.render(0, 4, out);
    EXPECT_EQ(out, std::vector<Sample>({10, 20, 30, 40}));

    feed.render(4, 4, out);
    EXPECT_EQ(out, std::vector<Sample>({0, 0, 0, 0}));
    EXPECT_FALSE(feed.finished());

    feed.render(8, 4, out);
    EXPECT_EQ(out, std::vector<Sample>({50, 60, 70, 80}));
    EXPECT_TRUE(feed.finished());
}

TEST(FeedTest, DropsLateSamples)
{
    Timeline timeline;
    timeline.streamStart = 0;

    Feed feed(400, 400, 0);
    std::vector<Sample> out;
    feed.render(0, 4, out);

    feed.queue().push({0, {10, 20, 30, 40}});
    feed.render(4, 4, out);

    EXPECT_EQ(out, std::vector<Sample>({0, 0, 0, 0}));
    EXPECT_EQ(feed.droppedSamples(), 4);
}

TEST(FeedTest, InterpolatesToHigherRate)
{
    Feed feed(800, 1600, 0);
    feed.queue().push({0, {0, 100, 200}});

    std::vector<Sample> out;
    feed.render(0, 4, out);

    EXPECT_EQ(out, std::vector<Sample>({0, 50, 100, 150}));
}

} // namespace
} // namespace AudioMixer
