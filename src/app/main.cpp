#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "app/Pipeline.hpp"

int main() {
  using namespace AudioMixer;

  try {
    // ------ DEMO APP SETUP -----
    const std::vector<std::string> feedPaths = {
        "data/feed0.bc", "data/feed1.bc", "data/feed2.bc"};

    const std::string outPath = "data/out.bc";
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

    runPipeline(feedPaths, volumePath, outPath, outputFormat);

    std::cout << "wrote output to : " << outPath << std::endl;
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