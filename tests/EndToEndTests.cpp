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

TEST(EndToEndTest, TestDifferentSampleRatesAndInterpolation) {
  const Sample unit = 1 << 16;
  const AudioFormat outputFormat{16000, 16};
  const std::vector<std::string> feedPaths = {"feed0.bc", "feed1.bc",
                                              "feed2.bc"};

  // 8 kHz ramp 0, 2, 4, ... upsampled to 16 kHz becomes 0, 1, 2, 3, ...
  writeRampAudioBlocks(feedPaths[0], start, 10, {8000, 16}, 2, 2 * unit);
  // constant feeds at 44.1 kHz and 96 kHz (32 bit)
  writeConstantAudioBlocks(feedPaths[1], start, 10, {44100, 16},
                           {100 * unit, 100 * unit});
  writeConstantAudioBlocks(feedPaths[2], start, 10, {96000, 32},
                           {-50 * unit, -50 * unit});

  { std::ofstream volume(volumePath); }  // all volumes stay at 100%

  runPipeline(feedPaths, volumePath, outPath, outputFormat);
  const AudioFile out = readAudioFile(outPath);

  ASSERT_EQ(out.blocks.size(), 2);
  std::vector<Sample> expected;

  // first 10 ms block at 16 kHz is 160 samples: k + 100 - 50
  for (Sample k = 0; k < 320; ++k) expected.push_back((k + 50) * unit);

  // the last sample interpolates towards silence: no input after the ramp ends
  // output: 159 + 100 - 50 = 209  instead of 369
  expected.back() = 209 * unit;

  std::vector<Sample> actual = out.blocks[0].samples;
  actual.insert(actual.end(), out.blocks[1].samples.begin(),
                out.blocks[1].samples.end());

  EXPECT_EQ(actual, expected);

  for (const auto& path : feedPaths) std::remove(path.c_str());

  std::remove(volumePath.c_str());
  std::remove(outPath.c_str());
}

}  // namespace AudioMixer