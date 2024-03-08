#include "rt/vec3.h"
#include <cmath>

namespace rt {

vec3::vec3() : _e{0, 0, 0} {}

vec3::vec3(float x, float y, float z) : _e{x, y, z} {}

vec3::~vec3() = default;

float vec3::x() const {
  return _e[0];
}

float vec3::y() const {
  return _e[1];
}

float vec3::z() const {
  return _e[2];
}

float vec3::length() const {
  return std::sqrt(length_squared());
}

float vec3::length_squared() const {
  return _e[0] * _e[0] + _e[1] * _e[1] + _e[2] * _e[2];
}

vec3 vec3::operator-() const {
  return vec3(-_e[0], -_e[1], -_e[2]);
}

float vec3::operator[](int i) const {
  return _e[i];
}

float& vec3::operator[](int i) {
  return _e[i];
}

vec3 vec3::operator+=(const vec3& v) {
  return vec3(_e[0] + v._e[0], _e[1] + v._e[1], _e[2] + v._e[2]);
}

vec3 vec3::operator-=(const vec3& v) {
  return vec3(_e[0] - v._e[0], _e[1] - v._e[1], _e[2] - v._e[2]);
}

vec3 vec3::operator*=(const float s) {
  return vec3(_e[0] * s, _e[1] * s, _e[2] * s);
}

vec3 vec3::operator/=(const float s) {
  return vec3(_e[0] / s, _e[1] / s, _e[2] / s);
}

}  // namespace rt