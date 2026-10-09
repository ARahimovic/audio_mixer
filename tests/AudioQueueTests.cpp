#include <gtest/gtest.h>

#include "core/AudioQueue.hpp"

namespace AudioMixer {

TEST(AudioQueueTest, PushAndPop) {
  AudioQueue queue;
  queue.push({1000, {1, 2}});
  queue.push({2000, {3}});

  std::optional<AudioBlock> first = queue.tryPop();
  std::optional<AudioBlock> second = queue.tryPop();

  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());

  EXPECT_EQ(first->startTime, 1000);
  EXPECT_EQ(second->startTime, 2000);
  EXPECT_FALSE(queue.tryPop().has_value());

  queue.close();
  EXPECT_TRUE(queue.closedAndEmpty());
}

TEST(AudioQueueTest, CheckClosedAndEmpty) {
  AudioQueue queue;
  EXPECT_FALSE(queue.closedAndEmpty());
  queue.push({1000, {1}});
  queue.close();
  EXPECT_FALSE(queue.closedAndEmpty());
  queue.tryPop();
  EXPECT_TRUE(queue.closedAndEmpty());
}

}  // namespace AudioMixer