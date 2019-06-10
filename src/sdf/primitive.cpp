#include "sdf/primitive.h"

primitive::cube_t::cube_t(const vec3_t & c, double s){
    this->c = c;
    this->s = s;
}

double
primitive::cube_t::phi(const vec3_t & x) const {
    vec3_t d = (x - c).map([&](double x){ 
        return std::max(std::abs(x) - s, 0.0); 
    }); 

    return d.norm() + std::min(d.chebyshev_norm(), 0.0);
}

primitive::sphere_t::sphere_t(const vec3_t & c, double r){
    this->c = c;
    this->r = r;
}

double
primitive::sphere_t::phi(const vec3_t & x) const {
    return (x - c).norm() - r;
}

vec3_t
primitive::sphere_t::normal(const vec3_t & x) const {
    return (x - c).normalise();
}

double
primitive::floor_t::phi(const vec3_t & x) const {
    return x[1];
}

vec3_t
primitive::floor_t::normal(const vec3_t & x) const {
    return vec3_t(0.0, 1.0, 0.0);
}