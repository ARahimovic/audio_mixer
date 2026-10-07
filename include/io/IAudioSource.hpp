#pragma once
#include "core/AudioTypes.hpp"

#include <optional>

namespace AudioMixer
{
    class IAudioSource
    {
        public :
        virtual ~IAudioSource() = default;

        virtual AudioFormat getFormat() const = 0;
        virtual std::optional<AudioBlock> read() = 0;
    };
}
