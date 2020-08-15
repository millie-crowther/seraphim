#include "substance/matter/matter.h"


matter_t::matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position){
    this->sdf = sdf;
    this->material = material;
    this->transform.set_position(initial_position);
}

std::shared_ptr<sdf3_t>
matter_t::get_sdf() const {
    return sdf;
}

vec3_t 
matter_t::get_position() const {
    return transform.get_position();
}

double 
matter_t::phi(const vec3_t & x) const {
    return sdf->phi(transform.to_local_space(x));
}

material_t
matter_t::get_material(const vec3_t & x){
    return material;
}

double
matter_t::get_average_density(){
    return material.density;
}

double
matter_t::get_mass(){
    return get_average_density() * sdf->get_volume();
}

transform_t & 
matter_t::get_transform(){
    return transform;
}

void 
matter_t::apply_force(const vec3_t & force){
    acceleration += force / get_mass();
}

void
matter_t::physics_tick(double delta){

}