find_package(GTest REQUIRED)

add_executable(unit_tests
    TimeTests.cpp
    VolumeTests.cpp
    FilterChainTests.cpp
    BadCodecReaderTests.cpp
    BadCodecWriterTests.cpp
    FileAudioSourceTests.cpp
    FileAudioSinkTests.cpp
    FileVolumeControllerTests.cpp
    AudioQueueTests.cpp
    FeedTests.cpp
    MixerTests.cpp
    EndToEndTests.cpp
)

target_link_libraries(unit_tests PRIVATE audiomixer GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)
