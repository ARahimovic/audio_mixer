#include "core/VolumeFilter.hpp"


namespace AudioMixer
{

    VolumeFilter::VolumeFilter(const VolumeTable& volumeTable, std::size_t feedId):m_volumeTable(volumeTable), m_feedId(feedId)
    {

    }

    void VolumeFilter::applyFilter(std::vector<Sample>& samples)
    {
        int volume = m_volumeTable.getVolume(m_feedId);
        if(volume == 100)
            return; //no change

        for(auto& sample : samples)
        {
            sample = static_cast<Sample>((static_cast<std::int64_t>(sample) * volume / 100));
        }
    }
}