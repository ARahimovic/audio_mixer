#include "audioEngine/AudioEngine.hpp"

#include <thread>

namespace AudioMixer {
AudioEngine::AudioEngine(std::vector<std::unique_ptr<IAudioSource>>& sources,
                         IAudioSink& sink, Mixer& mixer,
                         const Timeline& timeline, std::uint32_t outputRate,
                         std::chrono::milliseconds period)
    : m_sources(sources),
      m_mixer(mixer),
      m_sink(sink),
      m_timeline(timeline),
      m_outputRate(outputRate),
      m_period(period) {}

void AudioEngine::run() {
  std::vector<std::thread> readers;
  for (std::size_t i = 0; i < m_sources.size(); ++i) {
    readers.emplace_back(
        [source = m_sources[i].get(), queue = &m_mixer.feed(i).queue()] {
          while (auto block = source->read()) queue->push(std::move(*block));

          // no more data to push. close queue
          queue->close();
        });
  }

  mixerLoop();
  // all feeds finished, queues are empty and closed
  // join the readers thread
  for (auto& reader : readers) reader.join();
}

void AudioEngine::mixerLoop() {
  const std::size_t samplesPerPeriod = static_cast<std::size_t>(
      static_cast<std::uint64_t>(m_outputRate) *
      static_cast<std::uint64_t>(m_period.count()) / 1000);

  Clock::time_point deadline = m_timeline.playTime(m_timeline.streamStart);

  std::int64_t outputIndex = 0;
  AudioBlock outputBlock;

  while (!m_mixer.finished()) {
    std::this_thread::sleep_until(deadline);

    outputBlock.startTime =
        timeOfSampleAtIndex(m_timeline.streamStart, outputIndex, m_outputRate);

    m_mixer.render(outputIndex, samplesPerPeriod, outputBlock.samples);

    m_sink.write(outputBlock);

    outputIndex += static_cast<std::int64_t>(samplesPerPeriod);
    deadline += m_period;
  }
}
}  // namespace AudioMixer