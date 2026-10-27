#include "app/Pipeline.hpp"

#include <iostream>

#include "audioEngine/AudioEngine.hpp"
#include "control/FileVolumeController.hpp"
#include "core/VolumeFilter.hpp"
#include "core/VolumeTable.hpp"
#include "io/FileAudioSink.hpp"
#include "io/FileAudioSource.hpp"
#include "parser/BadCodecReader.hpp"
#include "parser/BadCodecWriter.hpp"

namespace AudioMixer {
void runPipeline(const std::vector<std::string>& feedPaths,
                 const std::string& volumePath, const std::string& outPath,
                 const AudioFormat& outputFormat) {
  Timeline timeline;
  timeline.streamStart = earliestStart(feedPaths);

  std::vector<std::unique_ptr<IAudioSource>> sources;
  for (const auto& path : feedPaths)
    sources.push_back(std::make_unique<FileAudioSource>(path, timeline));

  VolumeTable volumes(sources.size());
  Mixer mixer;
  for (std::size_t i = 0; i < sources.size(); ++i) {
    mixer.addFeed(std::make_unique<Feed>(sources[i]->getFormat().sampleRate,
                                         outputFormat.sampleRate,
                                         timeline.streamStart));
    mixer.feed(i).filters().addFilter(
        std::make_unique<VolumeFilter>(volumes, i));
  }

  FileAudioSink sink(outPath, outputFormat);
  FileVolumeController volumeController(volumePath, volumes, timeline);
  AudioEngine engine(sources, sink, mixer, timeline, outputFormat.sampleRate);

  timeline.wallStart = Clock::now();
  volumeController.start();
  engine.run();
  volumeController.stop();
}

// read first block from each file and return the earliest start time
Micros earliestStart(const std::vector<std::string>& paths) {
  Micros earliest = std::numeric_limits<Micros>::max();
  for (const auto& path : paths) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
      throw std::runtime_error("Couldnt open file at path :" + path);

    BadCodecReader reader(file);
    auto block = reader.readAudioBlock();
    earliest = std::min(earliest, block->startTime);
  }
  return earliest;
}

void writeConstantAudioBlocks(const std::string& path, Micros startTime,
                              std::size_t blockPeriod_ms,
                              const AudioFormat& outputFormat,
                              const std::vector<Sample>& values) {
  const std::size_t samplesPerBlock =
      outputFormat.sampleRate * blockPeriod_ms / 1000;
  std::ofstream file(path, std::ios::binary);
  BadCodecWriter writer(file, outputFormat);

  Micros time = startTime;
  // size of vector samples , determine how many blocks of period blockPeriod_ms
  // we generate
  for (const auto& value : values) {
    AudioBlock block{time, std::vector<Sample>(samplesPerBlock, value)};
    writer.writeAudioBlock(block);
    time += blockPeriod_ms * 1000;
  }
}

void writeRampAudioBlocks(const std::string& path, Micros startTime,
                          std::size_t blockPeriod_ms,
                          const AudioFormat& outputFormat,
                          std::size_t blockCount, Sample step) {
  const std::size_t samplesPerBlock =
      outputFormat.sampleRate * blockPeriod_ms / 1000;
  std::ofstream file(path, std::ios::binary);
  BadCodecWriter writer(file, outputFormat);

  Micros time = startTime;
  Sample value = 0;
  // the ramp continues across blocks: 0, step, 2*step, ...
  for (std::size_t i = 0; i < blockCount; ++i) {
    AudioBlock block{time, std::vector<Sample>(samplesPerBlock)};
    for (auto& sample : block.samples) {
      sample = value;
      value += step;
    }
    writer.writeAudioBlock(block);
    time += blockPeriod_ms * 1000;
  }
}

void printAudioFileContent(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  BadCodecReader reader(file);

  const auto format = reader.getFormat();
  const std::int32_t scale = format.bitPerSample == 16  ? 16
                             : format.bitPerSample == 8 ? 24
                                                        : 0;

  std::cout << path << ": " << format.sampleRate << " Hz, "
            << format.bitPerSample << " bit\n";

  std::size_t index = 0;
  while (auto block = reader.readAudioBlock()) {
    std::cout << "  block " << index++ << ", time=" << block->startTime
              << ", samples=" << block->samples.size() << std::endl;
    ;

    std::cout << "----- Samples ------ : " << std::endl;

    std::size_t sampleIndex = 0;
    for (const auto& sample : block->samples) {
      std::cout << "sample " << sampleIndex++ << ": " << sample / (1 << scale)
                << std::endl;
    }
  }
}

AudioFile readAudioFile(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) throw std::runtime_error("cannot open file: " + path);

  BadCodecReader reader(file);
  AudioFile result{reader.getFormat(), {}};
  while (auto block = reader.readAudioBlock())
    result.blocks.push_back(std::move(*block));

  return result;
}

}  // namespace AudioMixer