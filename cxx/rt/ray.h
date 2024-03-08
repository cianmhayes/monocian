#ifndef CXX_RT_RAY_H_
#define CXX_RT_RAY_H_

#include "vec3.h"

namespace rt {

class ray {
 public:
  ray();
  ray(const vec3& origin, const vec3& direction);

  const vec3& origin() const;
  const vec3& direction() const;

  vec3 at(float t) const;

 private:
  vec3 _origin;
  vec3 _direction;
};

}  // namespace rt

#endif  // CXX_RT_RAY_H_