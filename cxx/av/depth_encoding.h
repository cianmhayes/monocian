#ifndef CXX_AV_DEPTH_ENCODING_H
#define CXX_AV_DEPTH_ENCODING_H

#include <vector>

namespace av {

std::vector<uint8_t> DepthToRGB(const uint8_t* raw_depth_data,
                                size_t width,
                                size_t height);

std::vector<uint16_t> RGBToDepth(const uint8_t* raw_depth_data,
                                size_t width,
                                size_t height);

}  // namespace av

#endif  // CXX_AV_DEPTH_ENCODING_H