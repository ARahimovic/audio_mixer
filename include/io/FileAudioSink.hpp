#pragma once

#include <fstream>

#include "io/IAudioSink.hpp"
#include "parser/BadCodecWriter.hpp"

namespace AudioMixer {
class FileAudioSink : public IAudioSink {
 public:
  FileAudioSink(const std::string& path, const AudioFormat& audioFormat);
  void write(const AudioBlock& block) override;

 private:
  std::ofstream m_file;
  BadCodecWriter m_writer;
};
}  // namespace AudioMixer