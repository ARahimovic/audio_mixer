#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include "core/Mixer.hpp"
#include "core/Time.hpp"
#include "io/IAudioSink.hpp"
#include "io/IAudioSource.hpp"

namespace AudioMixer {
class AudioEngine {
 public:
  AudioEngine(std::vector<std::unique_ptr<IAudioSource>>& sources,
              IAudioSink& sink, Mixer& mixer, const Timeline& timeline,
              std::uint32_t outputRate,
              std::chrono::milliseconds period = std::chrono::milliseconds(10));

  void run();

 private:
  void mixerLoop();

  std::vector<std::unique_ptr<IAudioSource>>& m_sources;
  Mixer& m_mixer;
  IAudioSink& m_sink;
  const Timeline& m_timeline;
  std::uint32_t m_outputRate;
  std::chrono::milliseconds m_period;
};
}  // namespace AudioMixer