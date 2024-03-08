#ifndef CXX_RT_GEOMETRY_SPHERE_H_
#define CXX_RT_GEOMETRY_SPHERE_H_

#include "rt/hittable.h"

namespace rt {

class sphere : public hittable {
 public:
  sphere(const vec3& center, float radius) : _center(center), _radius(radius) {}

  std::vector<hit_record> hits(const ray& r,
                               float t_min,
                               float t_max) const override {
    rt::vec3 origin_to_center = r.origin() - _center;

    auto a = r.direction().length_squared();
    auto half_b = rt::dot(origin_to_center, r.direction());
    auto c = origin_to_center.length_squared() - _radius * _radius;

    hit_record hr;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) {
      return {hr};
    }

    // find nearest root in the acceptable range
    auto sqrt_d = std::sqrt(discriminant);
    auto root = (-half_b - sqrt_d) / a;
    if (root < t_min || root > t_max) {
      root = (-half_b + sqrt_d) / a;
      if (root < t_min || root > t_max) {
        return {hr};
      }
    }

    hr._hit = true;
    hr._t = root;
    hr._p = r.at(root);
    hr.set_face_normal(r, (hr._p - _center) / _radius);
    return {hr};
  }

 private:
  vec3 _center;
  float _radius;
};

}  // namespace rt

#endif  // CXX_RT_GEOMETRY_SPHERE_H_