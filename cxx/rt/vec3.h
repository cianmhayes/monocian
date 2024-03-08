#ifndef CXX_RT_VEC3_H_
#define CXX_RT_VEC3_H_

#include <iostream>

namespace rt {

class vec3 {
 public:
  vec3();
  vec3(float x, float y, float z);
  ~vec3();

  float x() const;
  float y() const;
  float z() const;

  float length() const;
  float length_squared() const;

  vec3 operator-() const;
  float operator[](int i) const;
  float& operator[](int i);

  vec3 operator+=(const vec3& v);
  vec3 operator-=(const vec3& v);
  vec3 operator*=(const float s);
  vec3 operator/=(const float s);

 public:
  float _e[3];
};

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << v._e[0] << ' ' << v._e[1] << ' ' << v._e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
  return vec3(u._e[0] + v._e[0], u._e[1] + v._e[1], u._e[2] + v._e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
  return vec3(u._e[0] - v._e[0], u._e[1] - v._e[1], u._e[2] - v._e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
  return vec3(u._e[0] * v._e[0], u._e[1] * v._e[1], u._e[2] * v._e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
  return vec3(t * v._e[0], t * v._e[1], t * v._e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
  return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
  return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
  return u._e[0] * v._e[0] + u._e[1] * v._e[1] + u._e[2] * v._e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
  return vec3(u._e[1] * v._e[2] - u._e[2] * v._e[1],
              u._e[2] * v._e[0] - u._e[0] * v._e[2],
              u._e[0] * v._e[1] - u._e[1] * v._e[0]);
}

inline vec3 unit_vector(const vec3& v) {
  return v / v.length();
}

}  // namespace rt

#endif  // CXX_RT_VEC3_H_