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

void 
matter_t::calculate_centre_of_mass(){
    double total_density;
    vec3_t total_mass = vec3_t(0.0);
    uint32_t sample = 0;

    while (sample < number_of_samples){
        vec3_t x = sdf->get_aabb().random();
        if (sdf->contains(x)){
            double density = get_material(x).density;
            total_density += density;
            total_mass += x * density;
            sample++;
        }
    }

    average_density = std::make_unique<double>(total_density / number_of_samples);
    centre_of_mass = std::make_unique<vec3_t>(total_mass / total_density);
}

vec3_t
matter_t::get_centre_of_mass(){
    if (!centre_of_mass){
        calculate_centre_of_mass();
    }

    return *centre_of_mass;
}

double
matter_t::get_average_density(){
    if (!average_density){
        calculate_centre_of_mass();
    }

    return *average_density;
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
    a += force / get_mass();
}

void
matter_t::physics_tick(double t){
    a[1] -= 9.8;
    transform.translate(a * 0.5 * t * t + v * t);
    v += a * t;
    a = vec3_t(0.0);
}