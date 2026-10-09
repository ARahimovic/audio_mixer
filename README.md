# Audio Mixer

Mixes several BadCodec audio feeds into one output, in real time.
Each feed has its own volume, which can change while audio is playing.

## Build and test

You need CMake 3.16+, a C++17 compiler and GoogleTest.

```sh
cmake -S . -B build
cmake --build build
cd build && ctest
```

## Run the demo

From the repository root:

```sh
mkdir -p data
./build/mixer
```

The demo creates three 20 ms feeds and a volume script in `data/`, mixes them, writes `data/out.bc`, and prints its samples.

| Feed | 0–10 ms | 10–20 ms |
|------|---------|----------|
| feed 0 | 10 | 10 |
| feed 1 | -10 | 10 |
| feed 2 | 5 | 10 |

Each line of the volume script is `time feed-index volume-percent`. At 9 ms it turns feed 2 down to 10%:

```text
2026-10-09T00:00:00.009 2 10
```

Expected output:

- **0–10 ms:** `10 - 10 + 5 = 5`
- **10–20 ms:** `10 + 10 + (10 × 10%) = 21`

## How it works

```text
source audio files ──► reader threads ──► queues ──► mixer thread ──► data/out.bc
                                                        ▲
                              volume script ──► volume thread
```

1. **Reading.** One thread per feed reads blocks from its file, waits until each block's timestamp, then pushes it to the feed's queue, as if it were arriving over a network.
2. **Mixing.** Every 10 ms the mixer thread wakes up and asks each feed for the next 10 ms of samples. Each feed resamples to the output rate with linear interpolation, applies its volume, and the mixer adds the feeds together.
3. **Writing.** The mixed block is written straight away, as if it were going to a speaker.
4. **Volume.** A separate thread reads the volume script and changes each volume at its scheduled time. It waits on a condition variable, so it can be stopped at once instead of sleeping until the next event.

## Design choices

- **Swappable parts.** The core only sees `IAudioSource`, `IAudioSink` and `IVolumeController`. A network source, a real audio output or an interactive volume control can replace the file versions without changing the mixer.
- **Any number of feeds.** The mixer just loops over its feeds; three is only the demo.
- **Filter chain.** Each feed has a list of filters. Volume is the first; more can be added.
- **One sample format inside.** All samples are converted to signed 32-bit on read, so 8-, 16- and 32-bit feeds can be mixed together.

## Supported formats

- Sample rates: 8, 16, 44.1, 48 and 96 kHz
- Sample sizes: 8, 16 and 32 bits
- Volume: 0–100% in 1% steps

## Assumptions and limits

- Output starts 20 ms after the stream so late blocks still have time to arrive. Samples that arrive after their play time are dropped.
- A volume change takes effect at the next 10 ms block.
- Integer math rounds toward zero, and the mixed sum is clamped to the 32-bit range.
- If the next block hasn't arrived, interpolation at the end of a block uses silence.

## Project layout

```text
include/, src/
  core/         timing, queues, feeds, filters, mixer
  audioEngine/  the orchestrator of threads and the real-time loop
  io/           file sources and sinks
  control/      volume script
  parser/       BadCodec reader and writer
  app/          pipeline wiring, file helpers and the demo
tests/          unit tests, plus end-to-end tests that run the full pipeline
```
