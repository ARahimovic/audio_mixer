#pragma once

#include "io/IAudioSink.hpp"
#include "parser/BadCodecWriter.hpp"
#include <fstream>

namespace AudioMixer 
{
    class FileAudioSink : public IAudioSink
    {
        public:
        FileAudioSink(const std::string& path, const AudioFormat& audioFormat);
        void write(const AudioBlock& block) override;

        private:
        std::ofstream m_file;
        BadCodecWriter m_writer;

    };
}