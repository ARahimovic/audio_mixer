#pragma once

#include <cstdint>

namespace AudioMixer
{
    using Micros = std::int64_t;
    constexpr Micros MICROS_IN_SECOND = 1000000;

    Micros toMicros(std::uint32_t seconds, std::uint32_t microSeconds);

    Micros timeOfSampleAtIndex(Micros start, std::int64_t index, std::uint32_t sampleRate);

    std::int64_t indexOfSampleAt(Micros start, Micros time, std::uint32_t sampleRate);

}
