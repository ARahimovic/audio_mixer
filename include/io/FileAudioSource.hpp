#pragma once
#include "core/Time.hpp"
#include "io/IAudioSource.hpp"
#include "parser/BadCodecReader.hpp"

#include <chrono>
#include <fstream>
#include <string>

namespace AudioMixer
{
    class FileAudioSource : public IAudioSource
    {
        public :
        FileAudioSource(const std::string& path, const Timeline& timeline,
                        std::chrono::milliseconds extraDelay =
                            std::chrono::milliseconds(0));
        AudioFormat getFormat() const override;
        std::optional<AudioBlock> read() override;

        private :
        std::ifstream m_file;
        BadCodecReader m_reader;
        const Timeline& m_timeline;
        std::chrono::milliseconds m_extraDelay;
    };
}