#pragma once
#include "core/AudioTypes.hpp"
#include "core/IFilter.hpp"
#include <memory>

namespace AudioMixer 
{
    class FilterChain
    {
        public:
        void addFilter(std::unique_ptr<IFilter> filter);
        void applyFilters(std::vector<Sample>& samples);
        std::size_t size() const;

        private :
        std::vector<std::unique_ptr<IFilter>> m_filters;
    };
}