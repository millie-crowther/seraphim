#include "sdf.h"

constexpr float sdf_t::epsilon;

sdf_t::sdf_t(std::function<float(const vec3_t&)> phi){
    this->phi = phi;
}

float
sdf_t::operator()(const vec3_t& p){
    return phi(p);
}

vec3_t
sdf_t::normal(const vec3_t& p){
    return vec3_t({
        phi(p + vec3_t({ epsilon, 0, 0 })) - phi(p - vec3_t({ epsilon, 0, 0 })),      
        phi(p + vec3_t({ 0, epsilon, 0 })) - phi(p - vec3_t({ 0, epsilon, 0 })),      
        phi(p + vec3_t({ 0, 0, epsilon })) - phi(p - vec3_t({ 0, 0, epsilon }))
    }).normalise();
}

bounds_t
sdf_t::get_bounds(){
    return bounds_t(vec3_t(), vec3_t());     
}
