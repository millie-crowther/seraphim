#include "sdf.h"

#include <chrono>
#include <iostream>
#include "bounds.h"

#include "maths/constants.h"


sdf_t::sdf_t(std::function<float(const vec3_t&)> phi){
    this->phi = phi;
}

double
sdf_t::operator()(const vec3_t& v) const {
    return phi(v);
}
 
vec3_t
sdf_t::normal(const vec3_t& p) const {
    return vec3_t({
        phi(p + vec3_t({ constants::epsilon, 0, 0 })) - phi(p - vec3_t({ constants::epsilon, 0, 0 })),      
        phi(p + vec3_t({ 0, constants::epsilon, 0 })) - phi(p - vec3_t({ 0, constants::epsilon, 0 })),      
        phi(p + vec3_t({ 0, 0, constants::epsilon })) - phi(p - vec3_t({ 0, 0, constants::epsilon }))
    }).normalise();
}

bounds_t
sdf_t::get_bounds() const {
    // TODO
    return bounds_t();
}
