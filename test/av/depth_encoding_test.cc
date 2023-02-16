#include <av/depth_encoding.h>

#include <gtest/gtest.h>

TEST(DepthEncodingTest, RoundTrip) {
    std::vector<uint16_t> expected_output = {1, 2, 257, 512, 0, 65535};
    std::vector<uint8_t> input = {};
    for (size_t i = 0; i < expected_output.size(); i++) {
        uint8_t high = (expected_output[i] & 0xFF00) >> 8;
        uint8_t low = (expected_output[i] & 0x00FF);
        input.push_back(high);
        input.push_back(low);
    }
    EXPECT_EQ(expected_output.size(), input.size() / 2);

    std::vector<uint8_t> encoded_depth = av::DepthToRGB(input.data(), 3, 2);
    std::vector<uint16_t> decoded_depth = av::RGBToDepth(encoded_depth.data(), 3, 2);
    EXPECT_EQ(expected_output.size(), decoded_depth.size());
    for (size_t i = 0; i < expected_output.size(); i++) {
        EXPECT_EQ(expected_output[i], decoded_depth[i]);
    }
}