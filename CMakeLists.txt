cmake_minimum_required(VERSION 3.16)
project(audio_mixer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(audiomixer STATIC
    src/core/Time.cpp
    src/core/VolumeTable.cpp
    src/core/VolumeFilter.cpp
    src/core/FilterChain.cpp
    src/parser/BadCodecReader.cpp
    src/parser/BadCodecWriter.cpp
    src/io/FileAudioSource.cpp
    src/io/FileAudioSink.cpp
)

target_include_directories(audiomixer PUBLIC include)
target_compile_options(audiomixer PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mixer src/app/main.cpp)
target_link_libraries(mixer PRIVATE audiomixer)

option(BUILD_TEST "enable Unit Tests" OFF)

if(BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()