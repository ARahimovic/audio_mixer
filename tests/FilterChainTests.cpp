#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "core/FilterChain.hpp"
#include "core/VolumeFilter.hpp"
#include "core/VolumeTable.hpp"

namespace AudioMixer {
namespace {

TEST(FilterChainTest, EmptyChain) {
  FilterChain chain;
  std::vector<Sample> samples = {1000, -1000};

  chain.applyFilters(samples);

  EXPECT_EQ(chain.size(), 0);
  EXPECT_EQ(samples, std::vector<Sample>({1000, -1000}));
}

TEST(FilterChainTest, fullChain) {
  VolumeTable volumes(2, 100);
  volumes.setVolume(0, 50);
  volumes.setVolume(1, 90);

  FilterChain chain;
  chain.addFilter(std::make_unique<VolumeFilter>(volumes, 0));
  chain.addFilter(std::make_unique<VolumeFilter>(volumes, 1));

  std::vector<Sample> samples = {100};
  chain.applyFilters(samples);

  // 100 * 50 / 100 = 50, then 50 * 90 / 100 = 45.
  EXPECT_EQ(chain.size(), 2);
  EXPECT_EQ(samples[0], 45);
}

}  // namespace
}  // namespace AudioMixer
