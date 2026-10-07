#pragma once

#include <vector>
#include "core/Time.hpp"
#include "core/AudioTypes.hpp"

namespace AudioMixer
{
    class IFilter 
    {
        public :
        virtual ~IFilter() = default;
        virtual void applyFilter(std::vector<Sample>& samples) = 0 ;
    };
}