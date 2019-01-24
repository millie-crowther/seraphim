#include "sdf.h"

#include <chrono>
#include <iostream>
#include "bounds.h"

#include "core/constant.h"


sdf_t::sdf_t(const std::function<float(const vec3_t&)>& phi){
    this->phi = phi;
}

double
sdf_t::operator()(const vec3_t& v) const {
    return phi(v);
}
 
vec3_t
sdf_t::normal(const vec3_t& p) const {
    return vec3_t({
        phi(p + vec3_t({ constant::epsilon, 0, 0 })) - phi(p - vec3_t({ constant::epsilon, 0, 0 })),      
        phi(p + vec3_t({ 0, constant::epsilon, 0 })) - phi(p - vec3_t({ 0, constant::epsilon, 0 })),      
        phi(p + vec3_t({ 0, 0, constant::epsilon })) - phi(p - vec3_t({ 0, 0, constant::epsilon }))
    }).normalise();
}

bounds_t
sdf_t::get_bounds() const {
    // TODO
    return bounds_t();
}
