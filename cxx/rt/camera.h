#ifndef CXX_RT_CAMERA_H
#define CXX_RT_CAMERA_H

#include <stdint.h>
#include "ray.h"
#include "vec3.h"

namespace rt {

class camera {
 public:
  camera()
      : _focal_length(1.0),
        _image_width(800),
        _aspect_ratio(16.0 / 9.0),
        _viewport_height(2.0) {
    _image_height = static_cast<int32_t>(_image_width / _aspect_ratio);
    _image_height = _image_height < 1 ? 1 : _image_height;

    _viewport_width =
        _viewport_height * (static_cast<float>(_image_width) / _image_height);

    _viewport_u = vec3(_viewport_width, 0, 0);
    _pixel_delta_u = _viewport_u / _image_width;

    _viewport_v = vec3(0, -_viewport_height, 0);
    _pixel_delta_v = _viewport_v / _image_height;

    _viewport_upper_left = _camera_center - vec3(0, 0, _focal_length) -
                           (_viewport_u / 2) - (_viewport_v / 2);
    _pixel00_loc =
        _viewport_upper_left + 0.5 * (_pixel_delta_u + _pixel_delta_v);
  }

  int32_t image_width() const { return _image_width; }
  int32_t image_height() const { return _image_height; }

  ray get_ray(int32_t x, int32_t y) const {
    auto pixel_center =
        _pixel00_loc + (x * _pixel_delta_u) + (y * _pixel_delta_v);
    auto ray_direction = pixel_center - _camera_center;
    return ray(_camera_center, ray_direction);
  }

 private:
  float _focal_length;
  vec3 _camera_center;
  vec3 _viewport_u;
  vec3 _viewport_v;
  vec3 _pixel_delta_u;
  vec3 _pixel_delta_v;

  int32_t _image_width;
  int32_t _image_height;
  float _aspect_ratio;

  float _viewport_height;
  float _viewport_width;
  vec3 _viewport_upper_left;

  vec3 _pixel00_loc;
};

}  // namespace rt

#endif CXX_RT_CAMERA_H