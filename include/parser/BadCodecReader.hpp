#pragma once

#include <istream>
#include <optional>

#include "core/AudioTypes.hpp"

namespace AudioMixer {
class BadCodecReader {
 public:
  explicit BadCodecReader(std::istream& in);
  AudioFormat getFormat() const;
  std::optional<AudioBlock> readAudioBlock();

 private:
  bool read16(std::uint16_t& value);
  bool read32(std::uint32_t& value);
  bool isValidFormat() const;

  AudioFormat m_audioFormat;
  std::istream& m_in;
};
}  // namespace AudioMixer