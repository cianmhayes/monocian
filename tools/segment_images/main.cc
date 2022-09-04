
#include <stb_image.h>
#include <stb_image_resize.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include "base/graph.h"
#include "selective_search/selective_search.h"

int main(int argc, char** argv) {
  int32_t width, height, channels;
  uint8_t* data = stbi_load(
      "C:\\code\\anna-atkins\\raw_scan_segmentation\\cache\\data\\unprocessed_"
      "1000px\\0aaf33267cb9174f7d8c28fcb0bac36d",
      &width, &height, &channels, 0);

  int32_t new_width = width / 3;
  int32_t new_height = height / 3;
  std::vector<uint8_t> resized(new_width * new_height * channels);
  stbir_resize_uint8(data, width, height, 0, resized.data(), new_width, new_height, 0, channels);

  std::vector<std::vector<uint8_t>> pixel_vector = {};
  for (int32_t i = 0; i < new_width * new_height * channels; i += channels) {
    std::vector<uint8_t> pixel = {};
    for (int32_t j = 0; j < channels; j++) {
      pixel.push_back(resized[i + j]);
    }
    pixel_vector.push_back(std::move(pixel));
  }
  std::unique_ptr<base::Graph> g =
      base::Graph::MakeGridGraph<std::vector<uint8_t>>(
          pixel_vector, new_width,
          [](const std::vector<uint8_t>& left,
             const std::vector<uint8_t>& right) {
            float distance = 0.0;
            for (size_t i = 0; i < left.size(); i++) {
              float delta = (left[i] - right[i]) / 255.0;
              distance += delta * delta;
            }
            return distance / left.size();
          });
  int near_zero_diff_count = 0;
  for (const auto& e : g->GetEdges()) {
    if (e.weight < 1e-06)
        near_zero_diff_count++;
  }

  auto components = selective_search::SelectiveSearch(
      *g.get(), [](const selective_search::Component& c) { return 0.5f / (c.component_size+1); });
  
  //std::cout << near_zero_diff_count << "/" << g->GetEdges().size() << "\n";
  return 0;
}