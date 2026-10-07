#include "core/VolumeTable.hpp"
#include <stdexcept>
#include <algorithm>

namespace AudioMixer
{

    VolumeTable::VolumeTable(std::size_t numFeeds, int initVolume):volumes(numFeeds)
    {
        for(std::size_t feed = 0; feed < volumes.size(); feed++)
        {
            setVolume(feed, initVolume);
        }
    }

    void VolumeTable::setVolume(std::size_t feedId, int volume)
    {
        if(feedId >= volumes.size())
            throw std::out_of_range("Feed ID out of range");

        volumes[feedId].store(std::clamp(volume, 0, 100));
    }

    int VolumeTable::getVolume(std::size_t feedId) const
    {
        if(feedId >= volumes.size())
            throw std::out_of_range("Feed ID out of range");

        return volumes[feedId].load();
    }

    size_t VolumeTable::size() const
    {
        return volumes.size();
    } 

}