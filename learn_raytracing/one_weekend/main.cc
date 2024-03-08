#include <iostream>
#include <vector>

#include <rt/camera.h>
#include <rt/geometry/sphere.h>
#include <rt/ray.h>
#include <rt/vec3.h>
#include <stb_image_write.h>

int save_png(char const* filename,
             int32_t w,
             int32_t h,
             const std::vector<rt::vec3>& image) {
  std::vector<uint8_t> data(3 * image.size());
  for (size_t i = 0; i < image.size(); i++) {
    data[i * 3] = static_cast<uint8_t>((image[i][0] * 255.999));
    data[i * 3 + 1] = static_cast<uint8_t>((image[i][1] * 255.999));
    data[i * 3 + 2] = static_cast<uint8_t>((image[i][2] * 255.999));
  }
  return stbi_write_png(filename, w, h, 3, data.data(), 3 * w);
};

float hit_sphere(const rt::ray& r) {
  rt::vec3 center(0, 0, -1);
  float radius = 0.5;
  rt::vec3 origin_to_center = r.origin() - center;

  auto a = r.direction().length_squared();
  auto half_b = rt::dot(origin_to_center, r.direction());
  auto c = origin_to_center.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - 4 * a * c;
  if (discriminant < 0) {
    return -1.0;
  } else {
    return (-half_b - std::sqrt(discriminant)) / a;
  }
}

rt::vec3 get_ray_color(const rt::ray& r) {
  auto sphere = rt::sphere(rt::vec3(0,0,-1), 0.5f);
  std::vector<rt::hit_record> hr = sphere.hits(r, 0.0, 100.0);
  if (hr.size() > 0 && hr[0]._hit) {
    return 0.5 * rt::vec3(hr[0]._normal.x() + 1, hr[0]._normal.y() + 1, hr[0]._normal.z() + 1);
  }
  rt::vec3 unit_direction = unit_vector(r.direction());
  auto a = 0.5 * (unit_direction.y() + 1);
  return (1.0 - a) * rt::vec3(1.0, 1.0, 1.0) + a * rt::vec3(0.5, 0.7, 1.0);
}

int main() {
  rt::camera cam;
  std::vector<rt::vec3> image(cam.image_width() * cam.image_height());
  for (int32_t y = cam.image_height() - 1; y >= 0; --y) {
    for (int32_t x = cam.image_width() - 1; x >= 0; --x) {
      rt::ray r = cam.get_ray(x, y);
      image[y * cam.image_width() + x] = get_ray_color(r);
    }
  }
  save_png("gradient.png", cam.image_width(), cam.image_height(), image);
}