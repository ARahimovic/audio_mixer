#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "audioEngine/AudioEngine.hpp"
#include "control/FileVolumeController.hpp"
#include "core/VolumeFilter.hpp"
#include "core/VolumeTable.hpp"
#include "io/FileAudioSink.hpp"
#include "io/FileAudioSource.hpp"
#include "parser/BadCodecReader.hpp"
#include "parser/BadCodecWriter.hpp"

namespace AudioMixer {

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

}  // namespace AudioMixer

int main() {
  using namespace AudioMixer;

  try {

    // ------ DEMO APP SETUP -----
    const std::vector<std::string> feedPaths = {
        "data/feed0.bc", "data/feed1.bc", "data/feed2.bc"};
    const std::string volumePath = "data/volume.txt";
    const AudioFormat outputFormat{8000, 16};

    Micros startime = 1791504000000000;  // october 9th 2026 00:00:00:00
    const std::int32_t scale = outputFormat.bitPerSample == 16  ? 16
                               : outputFormat.bitPerSample == 8 ? 24
                                                                : 0;
    // feed0 will have constant value from 0 - 20 ms
    writeConstantAudioBlocks(feedPaths[0], startime, 10, outputFormat,
                             {10 * 1 << scale, 10 * 1 << scale});
    // feed1 will have -10 from 0-10ms / 10 from 10-20ms
    writeConstantAudioBlocks(feedPaths[1], startime, 10, outputFormat,
                             {-10 * (1 << scale), 10 * 1 << scale});
    // feed2 will have 5 from 0-10ms / 10 from 10-20ms
    writeConstantAudioBlocks(feedPaths[2], startime, 10, outputFormat,
                             {5 * 1 << scale, 10 * 1 << scale});

    {
      std::ofstream volume(volumePath);
      volume << "2026-10-09T00:00:00.009 2 10\n";
    }
    // ------ DEMO APP SETUP -----

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

    const std::string outPath = "out.bc";
    FileAudioSink sink(outPath, outputFormat);
    std::unique_ptr<IVolumeController> volumeController =
        std::make_unique<FileVolumeController>(volumePath, volumes, timeline);

    AudioEngine engine(sources, sink, mixer, timeline, outputFormat.sampleRate);

    timeline.wallStart = Clock::now();
    volumeController->start();
    engine.run();
    volumeController->stop();

    std::cout << "wrote output to out.bc" << std::endl;
    // with currennt values
    // sum = 10 -10 + 5 = 5 from 0-10ms
    // volume applies to feed2 at 9ms, so become 1
    //  sum = 10 + 10 + 1 = 21
    printAudioFileContent(outPath);

    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
  }
}