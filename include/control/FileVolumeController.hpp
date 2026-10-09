#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "control/IVolumeController.hpp"
#include "core/Time.hpp"
#include "core/VolumeTable.hpp"

namespace AudioMixer {
struct VolumeEvent {
  Micros time;
  std::size_t feed;
  int percent;
};

// helper function to parse VolumeFile
VolumeEvent parseVolumeLine(const std::string& line);

class FileVolumeController : public IVolumeController {
 public:
  FileVolumeController(const std::string& path, VolumeTable& volTable,
                       const Timeline& timeline);
  ~FileVolumeController() override;

  void start() override;
  void stop() override;

 private:
  void run();
  bool waitUntil(Clock::time_point deadline);

  std::vector<VolumeEvent> m_events;
  VolumeTable& m_volumeTable;
  const Timeline& m_timeline;

  std::thread m_thread;
  std::mutex m_mutex;
  std::condition_variable m_wake;
  bool m_stopRequested = false;
};
}  // namespace AudioMixer
