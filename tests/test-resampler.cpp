#include <gtest/gtest.h>
#include "homein-resampler.hpp"
#include <vector>
#include <cmath>

/**
 * @brief Tests the HomeInResampler for accuracy and stability.
 * v1.1 Testing Framework
 */
TEST(ResamplerTest, HandlesSilence) {
    HomeInResampler resampler(48000, 16000);
    std::vector<float> input(1024, 0.0f);
    std::vector<float> output;
    
    // Resampling from 48k to 16k
    resampler.Process(input, output);
    
    ASSERT_FALSE(output.empty());
    for (float sample : output) {
        EXPECT_NEAR(sample, 0.0f, 0.0001f);
    }
}

// Verify that a constant DC offset is preserved (sanity check)
TEST(ResamplerTest, PreservesDCOffset) {
    HomeInResampler resampler(44100, 16000);
    std::vector<float> input(1024, 0.5f);
    std::vector<float> output;
    
    resampler.Process(input, output);
    
    // The first few samples might be transient, but the rest should be close to 0.5
    ASSERT_FALSE(output.empty());
    for (size_t i = 5; i < output.size(); ++i) {
        EXPECT_NEAR(output[i], 0.5f, 0.01f);
    }
}

// Verify output buffer sizing logic
TEST(ResamplerTest, CorrectOutputSize) {
    HomeInResampler resampler(48000, 16000);
    std::vector<float> input(480, 0.0f); // 10ms at 48kHz
    std::vector<float> output;
    
    resampler.Process(input, output);
    
    // Should be roughly 160 samples (1/3 of input)
    EXPECT_EQ(output.size(), 160);
}

