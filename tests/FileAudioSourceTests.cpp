#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

#include "io/FileAudioSource.hpp"
#include "parser/BadCodecWriter.hpp"

namespace AudioMixer {
namespace {

TEST(FileAudioSourceTest, ReadsAFile) {
  const std::string path = "FileAudioSource.bc";
  const AudioFormat format{48000, 16};
  const AudioBlock original{1000000, {-13107200, 0, 13107200}};

  {
    std::ofstream file(path, std::ios::binary);
    BadCodecWriter writer(file, format);
    writer.writeAudioBlock(original);
  }

  Timeline timeline;
  timeline.streamStart = original.startTime;
  timeline.wallStart = Clock::now();

  FileAudioSource source(path, timeline);
  const std::optional<AudioBlock> block = source.read();

  ASSERT_TRUE(block.has_value());
  EXPECT_EQ(source.getFormat().sampleRate, format.sampleRate);
  EXPECT_EQ(source.getFormat().bitPerSample, format.bitPerSample);
  EXPECT_EQ(block->startTime, original.startTime);
  EXPECT_EQ(block->samples, original.samples);

  std::remove(path.c_str());
}

TEST(FileAudioSourceTest, RejectsMissingFile) {
  Timeline timeline;

  EXPECT_THROW(FileAudioSource source("FileDoesNotExists.bc", timeline),
               std::runtime_error);
}

}  // namespace
}  // namespace AudioMixer
