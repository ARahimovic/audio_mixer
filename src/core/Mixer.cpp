#include "core/Mixer.hpp"

#include <algorithm>
#include <limits>

namespace AudioMixer {
void Mixer::addFeed(std::unique_ptr<Feed> feed) {
  m_feeds.push_back(std::move(feed));
}

Feed& Mixer::feed(std::size_t index) { return *m_feeds.at(index); }

std::size_t Mixer::feedCount() const { return m_feeds.size(); }

void Mixer::render(std::int64_t firstOutputIndex, std::size_t count,
                   std::vector<Sample>& out) {
  // reset sum at every period
  m_sum.assign(count, 0);
  m_feedSamples.resize(count);

  for (auto& feed : m_feeds) {
    feed->render(firstOutputIndex, count, m_feedSamples);
    for (std::size_t k = 0; k < count; ++k) m_sum[k] += m_feedSamples[k];
  }

  // after getting the sum. we clamp and store in output
  const std::int64_t lowest = std::numeric_limits<Sample>::min();
  const std::int64_t highest = std::numeric_limits<Sample>::max();
  out.resize(count);
  for (std::size_t k = 0; k < count; ++k)
    out[k] = static_cast<Sample>(std::clamp(m_sum[k], lowest, highest));
}

bool Mixer::finished() const {
  for (const auto& feed : m_feeds) {
    if (!feed->finished()) return false;
  }

  return true;
}
}  // namespace AudioMixer