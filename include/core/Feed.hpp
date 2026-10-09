#pragma once

#include "core/AudioQueue.hpp"
#include "core/AudioTypes.hpp"
#include "core/FilterChain.hpp"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>


namespace AudioMixer
{
    class Feed
    {
        public :
        Feed(std::uint32_t sourceRate, std::uint32_t outputRate, Micros streamStart);

        AudioQueue& queue();
        FilterChain& filters();

        //produce count samples at the output rate, starting at output sample firstOutputIndex
        void render(std::int64_t firstOutputIndex, std::size_t count, std::vector<Sample>& out);

        bool finished() const;
        std::int64_t droppedSamples() const;

        private :
        void insert(const AudioBlock& block);
        Sample sampleAt(std::int64_t index) const;

        AudioQueue m_queue;
        FilterChain m_filters;
        std::uint32_t m_sourceRate;
        std::uint32_t m_outputRate;
        Micros m_streamStart;
        
        //holds the samples to be played
        std::deque<Sample> m_buffer;
        // index of source sample that has not been played    
        std::int64_t m_bufferStart = 0;   
        std::int64_t m_droppedSamples = 0;

    };
}

