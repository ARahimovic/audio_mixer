#pragma once
#include "core/AudioTypes.hpp"
namespace AudioMixer {
class IAudioSink {
 public:
  virtual ~IAudioSink() = default;
  virtual void write(const AudioBlock& block) = 0;
};
}  // namespace AudioMixer