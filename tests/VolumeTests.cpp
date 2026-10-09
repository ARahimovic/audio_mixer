#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <vector>

#include "core/VolumeFilter.hpp"
#include "core/VolumeTable.hpp"

namespace AudioMixer {
namespace {

TEST(VolumeTableTest, InitialVolumeTest) {
  VolumeTable volumes(3, 100);

  EXPECT_EQ(volumes.size(), 3);
  EXPECT_EQ(volumes.getVolume(0), 100);
  EXPECT_EQ(volumes.getVolume(1), 100);
  EXPECT_EQ(volumes.getVolume(2), 100);
}

TEST(VolumeTableTest, ClampRangeTest) {
  VolumeTable volumes(2, 100);

  volumes.setVolume(0, 150);
  volumes.setVolume(1, -5);

  EXPECT_EQ(volumes.getVolume(0), 100);
  EXPECT_EQ(volumes.getVolume(1), 0);
}

TEST(VolumeTableTest, OutOfRangeTest) {
  VolumeTable volumes(2, 100);

  EXPECT_THROW(volumes.setVolume(2, 50), std::out_of_range);
  EXPECT_THROW(volumes.getVolume(2), std::out_of_range);
  EXPECT_EQ(volumes.size(), 2);
}

TEST(VolumeFilterTest, ApplyVolumeTest) {
  VolumeTable volumes(1, 13);
  VolumeFilter filter(volumes, 0);
  std::vector<Sample> samples = {-200};

  filter.applyFilter(samples);

  EXPECT_EQ(samples[0], -26);
}

TEST(VolumeFilterTest, ZeroProducesSilence) {
  VolumeTable volumes(1, 0);
  VolumeFilter filter(volumes, 0);
  std::vector<Sample> samples = {1000, -1000};

  filter.applyFilter(samples);

  EXPECT_EQ(samples, std::vector<Sample>({0, 0}));
}

TEST(VolumeFilterTest, FullVolumeTest) {
  VolumeTable volumes(1, 100);
  VolumeFilter filter(volumes, 0);
  std::vector<Sample> samples = {1000, -1000};

  filter.applyFilter(samples);

  EXPECT_EQ(samples, std::vector<Sample>({1000, -1000}));
}

TEST(VolumeFilterTest, UpdateVolumeTest) {
  VolumeTable volumes(1, 100);
  // works through IFilter interface
  std::unique_ptr<IFilter> filter = std::make_unique<VolumeFilter>(volumes, 0);
  std::vector<Sample> samples = {1000, -1000};

  filter->applyFilter(samples);
  EXPECT_EQ(samples, std::vector<Sample>({1000, -1000}));

  volumes.setVolume(0, 50);
  filter->applyFilter(samples);
  EXPECT_EQ(samples, std::vector<Sample>({500, -500}));
}

}  // namespace
}  // namespace AudioMixer
