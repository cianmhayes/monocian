#ifndef CXX_RT_HITTABLE_H_
#define CXX_RT_HITTABLE_H_

#include "ray.h"
#include "vec3.h"

namespace rt {

class hit_record {
 public:
  bool _hit = false;
  bool _front_face = false;
  float _t = 0.0;
  vec3 _p;
  vec3 _normal;

  void set_face_normal(const ray& r, const vec3& outward_normal) {
    _front_face = dot(r.direction(), outward_normal) < 0;
    _normal = _front_face ? outward_normal : - outward_normal;
  }
  
};

class hittable {
 public:
  //hittable() = default;
  virtual ~hittable() = default;

  virtual std::vector<hit_record> hits(const ray& r, float t_min, float t_max) const = 0;
};

}  // namespace rt

#endif  // CXX_RT_HITTABLE_H_