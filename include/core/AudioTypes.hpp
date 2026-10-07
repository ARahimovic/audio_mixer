#pragma once 

#include <cstdint>
#include <vector>
#include "core/Time.hpp"

namespace AudioMixer 
{

    //all samples be treated as int32_t
    using Sample = std::int32_t;

    struct AudioFormat
    {
        std::uint32_t sampleRate;
        std::uint16_t bitPerSample;
    };

    struct AudioBlock
    {
        Micros startTime;
        std::vector<Sample> samples;        
    };

}