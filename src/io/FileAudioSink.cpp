#include "io/FileAudioSink.hpp"

#include <stdexcept>

namespace AudioMixer {
std::ofstream& checkFileOpen(std::ofstream& file, const std::string& path) {
  if (!file.is_open())
    throw std::runtime_error("Failed to open file for writing " + path);

  return file;
}

FileAudioSink::FileAudioSink(const std::string& path,
                             const AudioFormat& audioFormat)
    : m_file(path, std::ios::binary),
      m_writer(checkFileOpen(m_file, path), audioFormat) {}

void FileAudioSink::write(const AudioBlock& block) {
  m_writer.writeAudioBlock(block);
  m_file.flush();
}
}  // namespace AudioMixer