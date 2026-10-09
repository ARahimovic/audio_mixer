#pragma once

#include "core/AudioTypes.hpp"

#include <deque>
#include <mutex>
#include <optional>

namespace AudioMixer
{
    class AudioQueue
    {
        public:
        void push(AudioBlock block);
        std::optional<AudioBlock> tryPop();
        void close();
        bool closedAndEmpty() const;

        private:
        mutable std::mutex m_mutex;
        std::deque<AudioBlock> m_blocks;
        bool m_closed = false;
    };
}