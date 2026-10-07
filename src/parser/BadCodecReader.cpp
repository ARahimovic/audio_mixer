#include "parser/BadCodecReader.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>

namespace AudioMixer
{
    const char magicHeader[] = "bad-codec";
    constexpr std::size_t MAGIC_HEADER_SIZE = sizeof(magicHeader);
    constexpr std::size_t BLOCK_HEADER_SIZE = 12;

    //helper method
    Sample decodeSample(const char* bytes, std::uint16_t bits)
    {
        if (bits == 8) {
            std::int8_t value;
            std::memcpy(&value, bytes, sizeof(value));
            return static_cast<Sample>(value) * (1 << 24);
        }

        if (bits == 16) {
            std::int16_t value;
            std::memcpy(&value, bytes, sizeof(value));
            return static_cast<Sample>(value) * (1 << 16);
        }

        std::int32_t value;
        std::memcpy(&value, bytes, sizeof(value));
        return value;
    }
        
    bool BadCodecReader::read16(std::uint16_t &value)
    {
        m_in.read(reinterpret_cast<char*>(&value), sizeof(value));
        return static_cast<bool>(m_in);
    }

    bool BadCodecReader::read32(std::uint32_t &value)
    {
        m_in.read(reinterpret_cast<char*>(&value), sizeof(value));
        return static_cast<bool>(m_in);
    }

    bool BadCodecReader::isValidFormat() const
    {
        const std::uint32_t sampleRate = m_audioFormat.sampleRate;
        const std::uint16_t bitPerSample = m_audioFormat.bitPerSample;

        return (sampleRate == 8000 || sampleRate == 16000 || sampleRate == 44100  || sampleRate == 48000 || sampleRate == 96000)
        && (bitPerSample == 8 || bitPerSample == 16 || bitPerSample == 32);  
    }

    BadCodecReader::BadCodecReader(std::istream& in):m_in(in)
    {
        char buffer[MAGIC_HEADER_SIZE] = {};
        m_in.read(buffer, MAGIC_HEADER_SIZE);
        if(!m_in || std::memcmp(buffer, magicHeader, MAGIC_HEADER_SIZE) != 0 )
        {
            throw std::runtime_error("not a BadCodec stream (bad magic)");
        }  


        if(!read32(m_audioFormat.sampleRate) || !read16(m_audioFormat.bitPerSample))
        {
            throw std::runtime_error("truncated BadCodec header");
        }

        if(!isValidFormat())
        {
            throw std::runtime_error("unsupported BadCodec format: " + std::to_string(m_audioFormat.sampleRate) + " Hz, "
            + std::to_string(m_audioFormat.bitPerSample) + " bits");
        }

    }

    std::optional<AudioBlock> BadCodecReader::readAudioBlock()
    {
        std::uint32_t seconds = {};
        std::uint32_t microSeconds = {};
        std::uint32_t blockSize = {};
        const std::uint32_t bytesPerSample = m_audioFormat.bitPerSample / 8;

        if(!read32(seconds) || !read32(microSeconds) || !read32(blockSize) || blockSize < BLOCK_HEADER_SIZE || (blockSize - BLOCK_HEADER_SIZE) % bytesPerSample != 0)
        {
            std::cerr << "BadCodec: invalid block header, stopping this stream\n";
            return std::nullopt;
        }
        
        
        //buffer to hold the raw bytes
        std::vector<char> bytes(blockSize - BLOCK_HEADER_SIZE);
        //read the entire remaining sample block into the buffer
        m_in.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        if(!m_in)
        {
            std::cerr << "BadCodec: truncated Audio block\n";
            return std::nullopt;
        }

        AudioBlock outBlock;
        outBlock.startTime = toMicros(seconds, microSeconds);
        const std::uint32_t numOfSamples = (blockSize - BLOCK_HEADER_SIZE) /bytesPerSample; 
        outBlock.samples.resize(numOfSamples);


        for (std::size_t i = 0; i < outBlock.samples.size(); ++i)
        {
         
            outBlock.samples[i] = decodeSample(&bytes[i*bytesPerSample], m_audioFormat.bitPerSample);
        }

        return outBlock;
    }
    AudioFormat BadCodecReader::getFormat() const
    {
        return m_audioFormat;
    }
}