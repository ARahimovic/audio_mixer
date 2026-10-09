#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser/BadCodecReader.hpp"
#include "parser/BadCodecWriter.hpp"

namespace AudioMixer {
namespace {

TEST(BadCodecWriterTest, CorrectWrite) {
  std::ostringstream output(std::ios::binary | std::ios::out);
  BadCodecWriter writer(output, AudioFormat{48000, 16});
  writer.writeAudioBlock(AudioBlock{1000002, {-13107200, 0, 13107200}});

  const char expected[] =
      "bad-codec\0"                // magic
      "\x80\xBB\x00\x00"           // sample rate: 48000 Hz
      "\x10\x00"                   // bits per sample: 16
      "\x01\x00\x00\x00"           // seconds: 1
      "\x02\x00\x00\x00"           // microseconds: 2
      "\x12\x00\x00\x00"           // block size: 12 + 6 bytes
      "\x38\xFF\x00\x00\xC8\x00";  // samples: -200, 0, 200

  EXPECT_EQ(output.str(), std::string(expected, sizeof(expected) - 1));
}

TEST(BadCodecWriterTest, WriterReaderLoop) {
  const AudioFormat format{48000, 16};
  const AudioBlock original{123456789, {-13107200, 0, 13107200}};

  std::ostringstream output(std::ios::binary | std::ios::out);
  BadCodecWriter writer(output, format);
  writer.writeAudioBlock(original);

  std::istringstream input(output.str(), std::ios::binary);
  BadCodecReader reader(input);
  const std::optional<AudioBlock> decoded = reader.readAudioBlock();

  ASSERT_TRUE(decoded.has_value());
  EXPECT_EQ(reader.getFormat().sampleRate, format.sampleRate);
  EXPECT_EQ(reader.getFormat().bitPerSample, format.bitPerSample);
  EXPECT_EQ(decoded->startTime, original.startTime);
  EXPECT_EQ(decoded->samples, original.samples);
}

TEST(BadCodecWriterTest, UnsupportedFormat) {
  std::ostringstream output(std::ios::binary | std::ios::out);

  EXPECT_THROW(BadCodecWriter writer(output, AudioFormat{48000, 24}),
               std::runtime_error);
}

}  // namespace
}  // namespace AudioMixer
