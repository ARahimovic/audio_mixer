#pragma once

namespace AudioMixer {
class IVolumeController {
 public:
  virtual ~IVolumeController() = default;
  virtual void start() = 0;
  virtual void stop() = 0;
};
}  // namespace AudioMixer