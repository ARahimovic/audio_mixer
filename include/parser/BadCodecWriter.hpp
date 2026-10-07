#pragma once

#include "core/AudioTypes.hpp"

#include <ostream>

namespace AudioMixer
{

class BadCodecWriter
{
public:
    BadCodecWriter(std::ostream& out, const AudioFormat& format);

    AudioFormat getFormat() const;
    void writeAudioBlock(const AudioBlock& block);

private:
    void write16(std::uint16_t value);
    void write32(std::uint32_t value);
    bool isValidFormat() const;

    std::ostream& m_out;
    AudioFormat m_audioFormat;
};

}
