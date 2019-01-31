#include "scene/warp_field.h"

// TODO: add read/write lock on psis

warp_field_t::warp_field_t(){
}

void
warp_field_t::add_warp_function(const warp_func_t& psi){
    psis.push_back(psi);
}

vec3_t
warp_field_t::operator()(const vec3_t& v){
    vec3_t psi_v;
    for (warp_func_t& psi : psis){
        psi_v += psi(v);
    }
    return psi_v;
} 

void
warp_field_t::serialize(std::vector<vec3_t>& psi, double size, double resolution){
    
}