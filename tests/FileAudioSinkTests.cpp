#include <gtest/gtest.h>

#include "io/FileAudioSink.hpp"
#include "parser/BadCodecReader.hpp"

namespace AudioMixer
{

    
    TEST(FileAudioSinkTest, WriteBlockThenReadIt)
{
    AudioFormat format {48000, 32};
    AudioBlock block = {toMicros(10,500), {100, 20, 30 }};
    const std::string path = "FileAudioSink.bc";

    {
        //scoped so that we close the file
        FileAudioSink sink(path, format);
        sink.write(block);
    }
    
    
    std::ifstream inputFile(path, std::ios::binary);
    BadCodecReader reader(inputFile);
    const auto readBlock = reader.readAudioBlock();

    ASSERT_TRUE(readBlock.has_value());
    EXPECT_EQ(reader.getFormat().sampleRate, format.sampleRate);
    EXPECT_EQ(reader.getFormat().bitPerSample, format.bitPerSample);
    EXPECT_EQ(readBlock->startTime, block.startTime);
    EXPECT_EQ(readBlock->samples, block.samples);
    std::remove(path.c_str());
}

}