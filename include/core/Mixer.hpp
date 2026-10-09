#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "core/Feed.hpp"

namespace AudioMixer {

class Mixer {
 public:
  void addFeed(std::unique_ptr<Feed> feed);
  Feed& feed(std::size_t index);
  std::size_t feedCount() const;

  void render(std::int64_t firstOutputIndex, std::size_t count,
              std::vector<Sample>& out);
  bool finished() const;

 private:
  std::vector<std::unique_ptr<Feed>> m_feeds;
  // will hold intermediate calculations,
  // delcare as member to not reallocate at each call
  std::vector<std::int64_t> m_sum;
  std::vector<Sample> m_feedSamples;
};

}  // namespace AudioMixer