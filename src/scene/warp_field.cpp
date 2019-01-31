#include "scene/warp_field.h"

warp_field_t::warp_field_t(){
    resolution = default_resolution;
    size = default_size;
}

void
warp_field_t::add_warp_function(const warp_func_t& psi){
    psis.push_back(psi);
}

void
warp_field_t::set_size(int size){
    this->size = size;
}

void 
warp_field_t::set_resolution(int resolution){
    this->resolution = resolution;
}

vec3_t
warp_field_t::operator()(const vec3_t& v){
    // TODO
    return vec3_t(); 
}