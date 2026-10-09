#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "control/FileVolumeController.hpp"

namespace AudioMixer {
namespace {

TEST(FileVolumeControllerTest, ParsesLine) {
  const VolumeEvent event = parseVolumeLine("2022-12-31T01:59:59.999 0 13");

  EXPECT_EQ(event.time, 1672451999999000);
  EXPECT_EQ(event.feed, 0);
  EXPECT_EQ(event.percent, 13);
}

TEST(FileVolumeControllerTest, ApplyVolumeEventsAtDiffTimes) {
  const std::string path = "VolumeScript.txt";
  {
    // volume events at different  times
    std::ofstream file(path);
    file << "2022-12-31T02:00:00.010 1 80\n"
         << "2022-12-31T02:00:00.020 0 50\n";
  }

  // stream starts at 02:00:00.000. No playback delay, so a +10 ms
  // stream event is applied 10 ms after wallStart.
  Timeline timeline;
  timeline.streamStart = 1672452000000000;
  timeline.playbackDelay = std::chrono::milliseconds(0);
  timeline.wallStart = Clock::now();

  VolumeTable volumes(2);
  FileVolumeController controller(path, volumes, timeline);
  controller.start();
  // volumes changes are not immediate, so we need to wait for them to be
  // applied
  EXPECT_EQ(volumes.getVolume(0), 100);
  EXPECT_EQ(volumes.getVolume(1), 100);
  // let the controller thread do it job before stopping it
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // feed 1 should have changed by now
  EXPECT_EQ(volumes.getVolume(1), 80);
  // feed 0 volume stays the same
  EXPECT_EQ(volumes.getVolume(0), 100);
  // let the controller thread do it job before stopping it
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  controller.stop();

  EXPECT_EQ(volumes.getVolume(0), 50);
  EXPECT_EQ(volumes.getVolume(1), 80);

  std::remove(path.c_str());
}

TEST(FileVolumeControllerTest, StopExitImmediately) {
  const std::string path = "VolumeScriptFuture.txt";
  {
    std::ofstream file(path);
    file << "2022-12-31T03:00:00.000 0 10\n";
  }

  // stream starts at 02:00:00, so the event is one hour away
  Timeline timeline;
  timeline.streamStart = 1672452000000000;
  timeline.wallStart = Clock::now();

  VolumeTable volumes(1);
  FileVolumeController controller(path, volumes, timeline);
  controller.start();

  // calling stop immdiately terminates the thread, does not wait until event
  controller.stop();

  EXPECT_EQ(volumes.getVolume(0), 100);

  std::remove(path.c_str());
}

TEST(FileVolumeControllerTest, RejectsMissingFile) {
  Timeline timeline;
  VolumeTable volumes(1);

  EXPECT_THROW(FileVolumeController controller("FileDoesNotExist.txt", volumes,
                                               timeline),
               std::runtime_error);
}

}  // namespace
}  // namespace AudioMixer
