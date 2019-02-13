#include "maths/vec.h"

vec3_t::vec3_t() : vec_t<3>(){}

vec3_t::vec3_t(double x) : vec_t<3>(x){}

vec3_t::vec3_t(double x, double y, double z) : vec_t<3>(x, y, z){}

vec3_t 
vec3_t::cross(const vec3_t & v) const {
    return { 
        xs[1] * v.xs[2] - xs[2] * v.xs[1],
        xs[2] * v.xs[0] - xs[0] * v.xs[2],
        xs[0] * v.xs[1] - xs[1] * v.xs[0]
    };
}

vec3_t 
vec3_t::operator%(const vec3_t & v) const {
    return cross(v);
}