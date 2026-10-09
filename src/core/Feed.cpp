#include "core/Feed.hpp"

#include <algorithm>
#include <optional>

namespace AudioMixer
{
    Feed::Feed(std::uint32_t sourceRate, std::uint32_t outputRate, Micros streamStart)
        : m_sourceRate(sourceRate), m_outputRate(outputRate), m_streamStart(streamStart)
    {
    }

    AudioQueue& Feed::queue()
    {
        return m_queue;
    }

    FilterChain& Feed::filters()
    {
        return m_filters;
    }

    bool Feed::finished() const
    {
        return m_queue.closedAndEmpty() && m_buffer.empty();
    }

    std::int64_t Feed::droppedSamples() const
    {
        return m_droppedSamples;
    }

    void Feed::insert(const AudioBlock& block)
    {
        //get source index of the sample
        std::int64_t index = indexOfSampleAt(m_streamStart, block.startTime, m_sourceRate);

        for(auto sample : block.samples)
        {
            std::int64_t pos = index++ - m_bufferStart;

            //already played
            if(pos < 0)
            {
                m_droppedSamples++;
                continue;
            }

            //a gap in the stream is filled with silence
            if (pos >= static_cast<std::int64_t>(m_buffer.size()))
            {
                m_buffer.resize(static_cast<std::size_t>(pos) + 1, 0);
            }

            m_buffer[static_cast<std::size_t>(pos)] = sample;
        }
    }

    Sample Feed::sampleAt(std::int64_t index) const
    {
        std::int64_t pos = index - m_bufferStart;
        if (pos < 0 || pos >= static_cast<std::int64_t>(m_buffer.size()))
            return 0;
        return m_buffer[static_cast<std::size_t>(pos)];
    }

    void Feed::render(std::int64_t firstOutputIndex, std::size_t count, std::vector<Sample>& out)
    {
        //drain the queue
        while(std::optional<AudioBlock> block = m_queue.tryPop())
        {
            insert(*block);
        }

        out.resize(count);

        for(std::size_t k = 0; k < count; ++k)
        {
            //transform output index into source index
            std::int64_t position = (firstOutputIndex + static_cast<std::int64_t>(k)) * m_sourceRate;
            std::int64_t index_0 = position / m_outputRate;
            std::int64_t fraction = position % m_outputRate;
            std::int64_t sample_0 = sampleAt(index_0);

            if (fraction == 0)
            {
                out[k] = static_cast<Sample>(sample_0);
                continue;
            }

            std::int64_t sample_1 = sampleAt(index_0 + 1);
            out[k] = static_cast<Sample>(sample_0 + (sample_1 - sample_0) * fraction / m_outputRate);
        }

        std::int64_t next = (firstOutputIndex + static_cast<std::int64_t>(count)) * m_sourceRate / m_outputRate;
        std::int64_t played = std::min(next - m_bufferStart, static_cast<std::int64_t>(m_buffer.size()));
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + played);
        m_bufferStart = next;

        m_filters.applyFilters(out);
    }

}
