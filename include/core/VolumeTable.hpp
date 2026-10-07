#pragma once
#include <vector>
#include <atomic>
#include <cstddef>

namespace AudioMixer 
{
    class VolumeTable
    {
        public:
        VolumeTable(std::size_t numFeeds, int initVolume = 100);
        int getVolume(std::size_t feedId) const;
        size_t size() const;
        void setVolume(std::size_t feedId, int volume);

        private :
        std::vector<std::atomic<int>> volumes;
    };
}