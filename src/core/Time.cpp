#include "core/Time.hpp"

namespace AudioMixer {
Micros toMicros(std::uint32_t seconds, std::uint32_t microSeconds) {
  return static_cast<Micros>(seconds) * MICROS_IN_SECOND + microSeconds;
}

Micros timeOfSampleAtIndex(Micros start, std::int64_t index,
                           std::uint32_t sampleRate) {
  return start + index * MICROS_IN_SECOND / sampleRate;
}

std::int64_t indexOfSampleAt(Micros start, Micros time,
                             std::uint32_t sampleRate) {
  Micros scaled = (time - start) * sampleRate;
  // to always round up instead of floor
  return (scaled + MICROS_IN_SECOND / 2) / MICROS_IN_SECOND;
}

Clock::time_point Timeline::arrivalTime(Micros time) const {
  return wallStart + std::chrono::microseconds(time - streamStart);
}

Clock::time_point Timeline::playTime(Micros time) const {
  return arrivalTime(time) + playbackDelay;
}

}  // namespace AudioMixer