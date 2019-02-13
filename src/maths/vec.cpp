#include "maths/vec.h"

vec3_t::vec3_t() : vec3_t(0) {}

vec3_t::vec3_t(double x) : vec3_t(x, x, x) {}

vec3_t::vec3_t(double x, double y, double z) : vec_t<3>(x, y, z){}

vec3_t 
vec3_t::cross(const vec3_t & v) const {
    return vec3_t(
        xs[1] * v.xs[2] - xs[2] * v.xs[1],
        xs[2] * v.xs[0] - xs[0] * v.xs[2],
        xs[0] * v.xs[1] - xs[1] * v.xs[0]
    );
}

vec3_t 
vec3_t::operator%(const vec3_t & v) const {
    return cross(v);
}