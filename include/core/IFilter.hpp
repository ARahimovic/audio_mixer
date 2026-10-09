#pragma once

#include <vector>

#include "core/AudioTypes.hpp"
#include "core/Time.hpp"

namespace AudioMixer {
class IFilter {
 public:
  virtual ~IFilter() = default;
  virtual void applyFilter(std::vector<Sample>& samples) = 0;
};
}  // namespace AudioMixer