#include <gtest/gtest.h>

#include <fstream>

#include "app/Pipeline.hpp"

namespace AudioMixer {

const std::string volumePath = "testVolume.txt";
const std::string outPath = "testOutput.bc";
const Micros start = 1791504000000000;  // 2026-10-09T00:00:00.000
const AudioFormat format{8000, 16};
const std::int32_t scale = format.bitPerSample == 16  ? 16
                           : format.bitPerSample == 8 ? 24
                                                      : 0;

TEST(EndToEndTest, ArbitiraryNumberOfFeeds) {
  std::size_t numOfFeeds = 5;
  std::vector<std::string> feedPaths(numOfFeeds, "feed");

  for (std::size_t i = 0; i < 5; i++) {
    feedPaths[i] += std::to_string(i) + ".bc";
    const Sample value = static_cast<Sample>(i + 1) << scale;
    writeConstantAudioBlocks(feedPaths[i], start, 10, format,
                             {value, 2 * value});
  }

  {
    std::ofstream volume(volumePath);
    volume << "2026-10-09T00:00:00.009 0 100\n";
  }

  runPipeline(feedPaths, volumePath, outPath, format);
  const AudioFile out = readAudioFile(outPath);
  // two output blocks of 10ms each
  ASSERT_EQ(out.blocks.size(), 2);
  // 8000 khz, so 80 samples per 10ms block
  EXPECT_EQ(out.blocks[0].samples,
            std::vector<Sample>(80, 15 * 1 << scale));  // 1+2+3+4+5
  EXPECT_EQ(out.blocks[1].samples,
            std::vector<Sample>(80, 30 * 1 << scale));  // 2+4+6+8+10

  for (const auto& path : feedPaths) std::remove(path.c_str());

  std::remove(volumePath.c_str());
  std::remove(outPath.c_str());
}

TEST(EndToEndTest, FeedStartingLaterIsSilentUntilItStarts) {
  const std::vector<std::string> feedPaths = {"feed0.bc", "feed2.bc",
                                              "feed3.bc"};
  writeConstantAudioBlocks(feedPaths[0], start, 10, format,
                           {10 * 1 << scale, 10 * 1 << scale});
  writeConstantAudioBlocks(feedPaths[1], start, 10, format,
                           {20 * 1 << scale, 20 * 1 << scale});
  writeConstantAudioBlocks(feedPaths[2], start + 10000, 10, format,
                           {5 * 1 << scale});  // starts at 10 ms

  {
    std::ofstream volume(volumePath);
    volume << "2026-10-09T00:00:00.009 0 100\n";
  }

  runPipeline(feedPaths, volumePath, outPath, format);
  const AudioFile out = readAudioFile(outPath);

  ASSERT_EQ(out.blocks.size(), 2);
  EXPECT_EQ(out.blocks[0].samples,
            std::vector<Sample>(80, 30 * 1 << scale));  // 10+20+0
  EXPECT_EQ(out.blocks[1].startTime, start + 10000);
  EXPECT_EQ(out.blocks[1].samples,
            std::vector<Sample>(80, 35 * 1 << scale));  // 10+20+5

  for (const auto& path : feedPaths) std::remove(path.c_str());

  std::remove(volumePath.c_str());
  std::remove(outPath.c_str());
}

}  // namespace AudioMixer