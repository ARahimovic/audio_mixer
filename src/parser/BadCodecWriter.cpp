#include "parser/BadCodecWriter.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace AudioMixer
{
namespace
{

const char MAGIC_HEADER[] = "bad-codec";
constexpr std::uint32_t BLOCK_HEADER_SIZE = 12;

void encodeSample(Sample sample, std::uint16_t bitsPerSample, char* bytes)
{
    if (bitsPerSample == 8)
    {
        const std::int8_t value =
            static_cast<std::int8_t>(sample / (1 << 24));
        std::memcpy(bytes, &value, sizeof(value));
        return;
    }

    if (bitsPerSample == 16)
    {
        const std::int16_t value =
            static_cast<std::int16_t>(sample / (1 << 16));
        std::memcpy(bytes, &value, sizeof(value));
        return;
    }

    std::memcpy(bytes, &sample, sizeof(sample));
}

}

BadCodecWriter::BadCodecWriter(std::ostream& out,
                               const AudioFormat& format)
    : m_out(out), m_audioFormat(format)
{
    if (!isValidFormat())
        throw std::runtime_error("unsupported BadCodec format");

    m_out.write(MAGIC_HEADER, sizeof(MAGIC_HEADER));
    write32(m_audioFormat.sampleRate);
    write16(m_audioFormat.bitPerSample);

    if (!m_out)
        throw std::runtime_error("failed to write BadCodec header");
}

AudioFormat BadCodecWriter::getFormat() const
{
    return m_audioFormat;
}

void BadCodecWriter::writeAudioBlock(const AudioBlock& block)
{
    if (block.startTime < 0)
        throw std::runtime_error("BadCodec timestamp cannot be negative");

    const std::size_t bytesPerSample = m_audioFormat.bitPerSample / 8;
    const std::size_t payloadSize = block.samples.size() * bytesPerSample;
    if (payloadSize >
        std::numeric_limits<std::uint32_t>::max() - BLOCK_HEADER_SIZE)
    {
        throw std::runtime_error("BadCodec block is too large");
    }

    const Micros seconds = block.startTime / MICROS_IN_SECOND;
    if (seconds > std::numeric_limits<std::uint32_t>::max())
        throw std::runtime_error("BadCodec timestamp is too large");

    std::vector<char> bytes(payloadSize);
    for (std::size_t i = 0; i < block.samples.size(); ++i)
    {
        encodeSample(block.samples[i], m_audioFormat.bitPerSample,
                     &bytes[i * bytesPerSample]);
    }

    write32(static_cast<std::uint32_t>(seconds));
    write32(static_cast<std::uint32_t>(
        block.startTime % MICROS_IN_SECOND));
    write32(static_cast<std::uint32_t>(
        BLOCK_HEADER_SIZE + payloadSize));
    m_out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));

    if (!m_out)
        throw std::runtime_error("failed to write BadCodec block");
}

void BadCodecWriter::write16(std::uint16_t value)
{
    m_out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void BadCodecWriter::write32(std::uint32_t value)
{
    m_out.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

bool BadCodecWriter::isValidFormat() const
{
    const std::uint32_t rate = m_audioFormat.sampleRate;
    const std::uint16_t bits = m_audioFormat.bitPerSample;
    return (rate == 8000 || rate == 16000 || rate == 44100 ||
            rate == 48000 || rate == 96000) &&
           (bits == 8 || bits == 16 || bits == 32);
}

}
