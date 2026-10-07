#include "io/FileAudioSource.hpp"

#include <stdexcept>
#include <thread>

namespace AudioMixer
{
namespace
{

std::ifstream& checkOpen(std::ifstream& file, const std::string& path)
{
    if (!file.is_open())
        throw std::runtime_error("cannot open file: " + path);
    return file;
}

}

FileAudioSource::FileAudioSource(const std::string& path,
                                 const Timeline& timeline,
                                 std::chrono::milliseconds extraDelay)
    : m_file(path, std::ios::binary),
      m_reader(checkOpen(m_file, path)),
      m_timeline(timeline),
      m_extraDelay(extraDelay)
{
}

AudioFormat FileAudioSource::getFormat() const
{
    return m_reader.getFormat();
}

std::optional<AudioBlock> FileAudioSource::read()
{
    std::optional<AudioBlock> block = m_reader.readAudioBlock();
    if (!block)
        return std::nullopt;

    std::this_thread::sleep_until(
        m_timeline.arrivalTime(block->startTime) + m_extraDelay);
    return block;
}

}