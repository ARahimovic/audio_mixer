#pragma once
#include "core/IFilter.hpp"
#include "core/VolumeTable.hpp"

namespace AudioMixer {
class VolumeFilter : public IFilter {
 public:
  VolumeFilter(const VolumeTable& volumeTable, std::size_t feedId);
  void applyFilter(std::vector<Sample>& samples) override;

 private:
  const VolumeTable& m_volumeTable;
  std::size_t m_feedId;
};
}  // namespace AudioMixer