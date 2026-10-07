#include <gtest/gtest.h>

#include "control/FileVolumeController.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>

namespace AudioMixer
{
namespace
{

TEST(FileVolumeControllerTest, ParsesLine)
{
    const VolumeEvent event = parseVolumeLine("2022-12-31T01:59:59.999 0 13");

    EXPECT_EQ(event.time, 1672451999999000);
    EXPECT_EQ(event.feed, 0);
    EXPECT_EQ(event.percent, 13);
}

TEST(FileVolumeControllerTest, VolumeEventsUnordered)
{
    const std::string path = "VolumeScript.txt";
    {
        //events are not in time order, the controller needs to apply them in order
        std::ofstream file(path);
        file << "2022-12-31T02:00:05.000 0 50\n"
             << "2022-12-31T02:00:02.000 0 10\n"
             << "2022-12-31T02:00:02.000 1 30\n"
             << "2022-12-31T02:00:02.000 1 80\n";
    }

    // stream starts at 02:00:10, so every event is already in the past
    Timeline timeline;
    timeline.streamStart = 1672452010000000;
    timeline.wallStart = Clock::now();

    VolumeTable volumes(2);
    FileVolumeController controller(path, volumes, timeline);
    controller.start();
    //let the controller thread do it job before stopping it
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    controller.stop();

    EXPECT_EQ(volumes.getVolume(0), 50);
    EXPECT_EQ(volumes.getVolume(1), 80);

    std::remove(path.c_str());
}

TEST(FileVolumeControllerTest, StopDoesNotWaitForFutureEvents)
{
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

    const Clock::time_point before = Clock::now();
    //calling stop immdiately terminates the thread, does not wait until event
    controller.stop();

    EXPECT_LT(Clock::now() - before, std::chrono::seconds(1));
    EXPECT_EQ(volumes.getVolume(0), 100);

    std::remove(path.c_str());
}

TEST(FileVolumeControllerTest, RejectsMissingFile)
{
    Timeline timeline;
    VolumeTable volumes(1);

    EXPECT_THROW(
        FileVolumeController controller("FileDoesNotExist.txt", volumes, timeline),
        std::runtime_error);
}

} // namespace
} // namespace AudioMixer
