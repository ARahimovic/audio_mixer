#include "core/AudioQueue.hpp"

namespace AudioMixer
{
    
    void AudioQueue::push(AudioBlock block)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_blocks.push_back(std::move(block));
    }

    std::optional<AudioBlock> AudioQueue::tryPop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_blocks.empty())
            return std::nullopt;

        AudioBlock block = std::move(m_blocks.front());
        m_blocks.pop_front();
        return block;
    }

    //producer thread call this when done
    void AudioQueue::close()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_closed = true;
    }
    //consumer thread check this 
    bool AudioQueue::closedAndEmpty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_closed && m_blocks.empty();
    }
}