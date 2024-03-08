#include "ray.h"

namespace rt {

ray::ray() {}

ray::ray(const vec3& origin, const vec3& direction) : _origin(origin), _direction(direction) {}

const vec3& ray::origin() const {
    return _origin;
}

const vec3& ray::direction() const {
    return _direction;
}

vec3 ray::at(float t) const {
    return _origin + (t * _direction);
}

} // namespace rt