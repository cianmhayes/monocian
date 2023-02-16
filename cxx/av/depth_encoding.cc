#include "depth_encoding.h"

namespace av {

std::vector<uint8_t> DepthToRGB(const uint8_t* raw_depth_data,
                                size_t width,
                                size_t height) {
  std::vector<uint8_t> encoded_depth_data(width * height * 3);
  for (int i = 0; i < (width * height) / 2; i++) {
    // We want to prepare an RGB frame from 16bit depth data so that it will
    // preserve as much detail as possible when converted to YUV.
    // Given a pair of 16bit depth pixels, split each into most and least
    // significant bytes. Set the green channel to the most significant bit
    // which will become Y. Then the even least significant bytes will form
    // the red channel, and the odd least significant bytes will form the
    // blue channel.

    // Input is 2 bytes per pixel, and we encoded two at a time.
    int o = i * 4;
    uint8_t g1 = raw_depth_data[o + 1];
    uint8_t g2 = raw_depth_data[o + 3];

    uint8_t r1 = raw_depth_data[o + 0];
    uint8_t r2 = raw_depth_data[o + 0];
    uint8_t b1 = raw_depth_data[o + 2];
    uint8_t b2 = raw_depth_data[o + 2];

    // Output is 3 bytes eper pixel, and we're still doing two at a time.
    o = i * 6;
    encoded_depth_data[o + 0] = r1;
    encoded_depth_data[o + 1] = g1;
    encoded_depth_data[o + 2] = b1;

    encoded_depth_data[o + 3] = r2;
    encoded_depth_data[o + 4] = g2;
    encoded_depth_data[o + 5] = b2;
  }
  return encoded_depth_data;
}

std::vector<uint16_t> RGBToDepth(const uint8_t* encoded_depth_data,
                                 size_t width,
                                 size_t height) {
  std::vector<uint16_t> raw_depth_data(width * height);
  for (int i = 0; i < width * height; i += 2) {
    // Input is 3 bytes per pixel and we're decoding two at a time.
    int o = i * 3;
    uint8_t r1 = encoded_depth_data[o + 0];
    uint8_t g1 = encoded_depth_data[o + 1];
    uint8_t b1 = encoded_depth_data[o + 2];

    uint8_t r2 = encoded_depth_data[o + 3];
    uint8_t g2 = encoded_depth_data[o + 4];
    uint8_t b2 = encoded_depth_data[o + 5];

    // Output is one 16 bit element per pixel and we're still decoding two at a
    // time.
    raw_depth_data[i] = (r1 << 8) | g1;
    raw_depth_data[i + 1] = (b2 << 8) | g2;
  }
  return raw_depth_data;
}

}  // namespace av
