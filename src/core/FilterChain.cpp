#include "core/FilterChain.hpp"

namespace AudioMixer {
void FilterChain::addFilter(std::unique_ptr<IFilter> filter) {
  m_filters.push_back(std::move(filter));
}

void FilterChain::applyFilters(std::vector<Sample>& samples) {
  if (m_filters.empty()) return;

  for (auto& filter : m_filters) filter->applyFilter(samples);
}

std::size_t FilterChain::size() const { return m_filters.size(); }
}  // namespace AudioMixer