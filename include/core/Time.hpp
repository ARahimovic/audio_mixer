#pragma once

#include <chrono>
#include <cstdint>

namespace AudioMixer
{
    using Micros = std::int64_t;
    constexpr Micros MICROS_IN_SECOND = 1000000;
    using Clock = std::chrono::steady_clock;

    Micros toMicros(std::uint32_t seconds, std::uint32_t microSeconds);

    Micros timeOfSampleAtIndex(Micros start, std::int64_t index, std::uint32_t sampleRate);

    std::int64_t indexOfSampleAt(Micros start, Micros time, std::uint32_t sampleRate);

    struct Timeline
    {
        Micros streamStart = 0;
        Clock::time_point wallStart;
        std::chrono::milliseconds playbackDelay{20};

        Clock::time_point arrivalTime(Micros time) const;
        Clock::time_point playTime(Micros time) const;
    };

}
