#pragma once
#include <optional>

#include "core/AudioTypes.hpp"

namespace AudioMixer {
class IAudioSource {
 public:
  virtual ~IAudioSource() = default;

  virtual AudioFormat getFormat() const = 0;
  virtual std::optional<AudioBlock> read() = 0;
};
}  // namespace AudioMixer
