#include "sdf.h"

#include <memory>

#include "core/constant.h"

sdf_t::sdf_t() : sdf_t([](const vec3_t & v){ return 1; }){}

sdf_t::sdf_t(const phi_t & phi){
    this->phi = phi;
}

double
sdf_t::operator()(const vec3_t & v) const {
    return phi(v);
}
 
vec3_t
sdf_t::normal(const vec3_t & p) const {
    return vec3_t(
        phi(p + vec3_t(constant::epsilon, 0, 0)) - phi(p - vec3_t(constant::epsilon, 0, 0)),      
        phi(p + vec3_t(0, constant::epsilon, 0)) - phi(p - vec3_t(0, constant::epsilon, 0)),      
        phi(p + vec3_t(0, 0, constant::epsilon)) - phi(p - vec3_t(0, 0, constant::epsilon))
    ).normalise();
}

sdf_t
sdf_t::operator&&(const sdf_t & sdf) const {
    auto phi1 = std::shared_ptr<phi_t>(phi);
    auto phi2 = std::make_shared(sdf.phi); 
    return sdf_t([phi1, phi2](const vec3_t & v){
        return std::max((*phi1)(v), (*phi2)(v));
    });
} 

sdf_t
sdf_t::operator||(const sdf_t & sdf) const {
    auto phi1 = std::make_shared(phi);
    auto phi2 = std::make_shared(sdf.phi);
    return sdf_t([phi1, phi2](const vec3_t & v){
        return std::min((*phi1)(v), (*phi2)(v));
    });
}

sdf_t
sdf_t::operator!() const {
    auto phi_ptr = std::make_shared(phi);
    return sdf_t([phi_ptr](const vec3_t & v){
        return -(*phi_ptr)(v);
    }); 
}

bool
sdf_t::is_null() const {
    
    return false;
}