#pragma once

#include <string>
#include <vector>

#include "core/AudioTypes.hpp"

namespace AudioMixer {

struct AudioFile {
  AudioFormat format;
  std::vector<AudioBlock> blocks;
};

AudioFile readAudioFile(const std::string& path);

void runPipeline(const std::vector<std::string>& feedPaths,
                 const std::string& volumePath, const std::string& outPath,
                 const AudioFormat& outputFormat);

void printAudioFileContent(const std::string& path);
void writeConstantAudioBlocks(const std::string& path, Micros startTime,
                              std::size_t blockPeriod_ms,
                              const AudioFormat& outputFormat,
                              const std::vector<Sample>& values);
void writeRampAudioBlocks(const std::string& path, Micros startTime,
                          std::size_t blockPeriod_ms,
                          const AudioFormat& outputFormat,
                          std::size_t blockCount, Sample step);

Micros earliestStart(const std::vector<std::string>& paths);

}  // namespace AudioMixer