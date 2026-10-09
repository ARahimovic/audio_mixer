#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser/BadCodecReader.hpp"

namespace AudioMixer {
namespace {

TEST(BadCodecReaderTest, ReadsCorrectSamples) {
  {
    // 8-bit samples: {-128, -1, 127}.
    const char raw[] =
        "bad-codec\0"       // magic
        "\x40\x1F\x00\x00"  // sample rate: 8000 Hz
        "\x08\x00"          // bits per sample: 8
        "\x01\x00\x00\x00"  // seconds: 1
        "\x02\x00\x00\x00"  // microseconds: 2
        "\x0F\x00\x00\x00"  // block size: 12-byte header + 3 bytes
        "\x80\xFF\x7F";     // samples: -128, -1, 127
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    BadCodecReader reader(input);
    const auto block = reader.readAudioBlock();

    ASSERT_TRUE(block.has_value());
    EXPECT_EQ(reader.getFormat().sampleRate, 8000);
    EXPECT_EQ(block->startTime, 1000002);
    // samples are converted to 32-bit signed integer
    const std::int32_t scale = 1 << (32 - 8);
    const std::vector<Sample> expected = {
        INT32_MIN, static_cast<std::int32_t>(-1) * scale,
        static_cast<std::int32_t>(127) * scale};
    EXPECT_EQ(block->samples, expected);
  }

  {
    // 16-bit samples: {-200, 0, 200}.
    const char raw[] =
        "bad-codec\0"                // magic
        "\x80\xBB\x00\x00"           // sample rate: 48000 Hz
        "\x10\x00"                   // bits per sample: 16
        "\x01\x00\x00\x00"           // seconds: 1
        "\x02\x00\x00\x00"           // microseconds: 2
        "\x12\x00\x00\x00"           // block size: 12 + 6 bytes
        "\x38\xFF\x00\x00\xC8\x00";  // samples: -200, 0, 200
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    BadCodecReader reader(input);
    const auto block = reader.readAudioBlock();

    ASSERT_TRUE(block.has_value());
    const std::int32_t scale = 1 << (32 - 16);
    const std::vector<Sample> expected = {
        static_cast<std::int32_t>(-200) * scale,
        static_cast<std::int32_t>(0) * scale,
        static_cast<std::int32_t>(200) * scale};
    EXPECT_EQ(block->samples, expected);
  }

  {
    // 32-bit samples: {INT32_MIN, -1, INT32_MAX}.
    const char raw[] =
        "bad-codec\0"        // magic
        "\x80\xBB\x00\x00"   // sample rate: 48000 Hz
        "\x20\x00"           // bits per sample: 32
        "\x01\x00\x00\x00"   // seconds: 1
        "\x02\x00\x00\x00"   // microseconds: 2
        "\x18\x00\x00\x00"   // block size: 12 + 12 bytes
        "\x00\x00\x00\x80"   // sample: INT32_MIN
        "\xFF\xFF\xFF\xFF"   // sample: -1
        "\xFF\xFF\xFF\x7F";  // sample: INT32_MAX
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    BadCodecReader reader(input);
    const auto block = reader.readAudioBlock();

    ASSERT_TRUE(block.has_value());
    // samples stays the same, as no conversion is needed
    EXPECT_EQ(block->samples, std::vector<Sample>({INT32_MIN, -1, INT32_MAX}));
  }
}

TEST(BadCodecReaderTest, RejectsInvalidFileHeaders) {
  {
    const char raw[] =
        "not-codec\0"       // invalid magic
        "\x80\xBB\x00\x00"  // sample rate: 48000 Hz
        "\x10\x00";         // bits per sample: 16
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    EXPECT_THROW(BadCodecReader reader(input), std::runtime_error);
  }

  {
    const char raw[] =
        "bad-codec\0"  // magic
        "\x80";        // incomplete sample-rate field
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    EXPECT_THROW(BadCodecReader reader(input), std::runtime_error);
  }

  {
    // Unsupported 24-bit format.
    const char raw[] =
        "bad-codec\0"       // magic
        "\x80\xBB\x00\x00"  // sample rate: 48000 Hz
        "\x18\x00";         // unsupported bits per sample: 24
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    EXPECT_THROW(BadCodecReader reader(input), std::runtime_error);
  }
}

TEST(BadCodecReaderTest, RejectsInvalidBlocks) {
  {
    // One payload byte cannot hold a complete 16-bit sample.
    const char raw[] =
        "bad-codec\0"       // magic
        "\x80\xBB\x00\x00"  // sample rate: 48000 Hz
        "\x10\x00"          // bits per sample: 16
        "\x01\x00\x00\x00"  // seconds: 1
        "\x00\x00\x00\x00"  // microseconds: 0
        "\x0D\x00\x00\x00"  // invalid block size: 12 + 1 byte
        "\x01";             // incomplete 16-bit sample
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    BadCodecReader reader(input);
    EXPECT_FALSE(reader.readAudioBlock().has_value());
  }

  {
    // Size 16 promises two 16-bit samples, but only one follows.
    const char raw[] =
        "bad-codec\0"       // magic
        "\x80\xBB\x00\x00"  // sample rate: 48000 Hz
        "\x10\x00"          // bits per sample: 16
        "\x01\x00\x00\x00"  // seconds: 1
        "\x00\x00\x00\x00"  // microseconds: 0
        "\x10\x00\x00\x00"  // block size: 12 + 4 bytes
        "\x07\x00";         // only one of two promised samples
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    BadCodecReader reader(input);
    EXPECT_FALSE(reader.readAudioBlock().has_value());
  }

  {
    // Header followed by a clean end of stream.
    const char raw[] =
        "bad-codec\0"       // magic
        "\x80\xBB\x00\x00"  // sample rate: 48000 Hz
        "\x10\x00";         // bits per sample: 16, then EOF
    std::istringstream input(std::string(raw, sizeof(raw) - 1),
                             std::ios::binary);
    BadCodecReader reader(input);
    EXPECT_FALSE(reader.readAudioBlock().has_value());
  }
}

}  // namespace
}  // namespace AudioMixer
