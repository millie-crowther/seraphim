#include "metaphysics/matter.h"

matter_t::matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform){
    this->sdf = sdf;
    this->material = material;
    this->transform.set_position(initial_position);
    this->is_uniform = is_uniform;

    if (initial_position[1] > -50.0){
        omega = vec3_t(0.25);
    }
}

std::shared_ptr<sdf3_t>
matter_t::get_sdf() const {
    return sdf;
}

vec3_t 
matter_t::get_position() const {
    return transform.get_position();
}

material_t
matter_t::get_material(const vec3_t & x){
    return material;
}

aabb3_t
matter_t::get_aabb() const {
    aabb3_t aabb;
    
    for (uint8_t i = 0; i < 8; i++){   
        vec3_t x = transform.to_global_space(sdf->get_aabb().get_vertex(i));
        aabb.capture_point(x);
    }

    return aabb;
}

double
matter_t::get_inverse_angular_mass(const vec3_t & r_global, const vec3_t & n_global){
    auto i1 = get_inverse_inertia_tensor();
    auto r = transform.to_local_space(r_global) - get_centre_of_mass();
    auto n = transform.get_rotation().inverse() * n_global;
    auto rn = vec::cross(r, n);

    return vec::dot(rn, i1 * rn);
}

void
matter_t::update_velocity(double jr, const vec3_t & r_global, const vec3_t & n_global){
    v += jr / get_mass() * n_global;
    
    auto i1 = get_inverse_inertia_tensor();
    auto r = transform.to_local_space(r_global);
    auto n = transform.get_rotation().inverse() * n_global;
    auto rn = i1 * vec::cross(r, n);
    rn = transform.get_rotation() * rn;

    // TODO: i1 is local space and rn is global space. need to fix!!!
//    omega += jr * rn; 
}

void 
matter_t::calculate_centre_of_mass(){
    throw std::runtime_error("Error: autocalc of centre of mass not yet implemented.");
}

vec3_t
matter_t::get_centre_of_mass(){
    if (is_uniform){
        return sdf->get_uniform_centre_of_mass();
    }

    if (!centre_of_mass){
        calculate_centre_of_mass();
    }

    return *centre_of_mass;
}

double
matter_t::get_average_density(){
    if (is_uniform){
        return get_material(vec3_t()).density;
    }

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
matter_t::physics_tick(double t){
    transform.translate(v * t);
    
    // update rotation
    transform.rotate(quat_t::euler_angles(omega * t));

    // integrate accelerations into velocities
    v += a * t;
//    omega += alpha * t;
    
    // reset accelerations
    a = vec3_t(0.0, -9.8, 0.0);
    alpha = vec3_t(0.0);
}

mat3_t 
matter_t::get_inverse_inertia_tensor(){
    if (!inverse_inertia_tensor){
        if (is_uniform){
            inverse_inertia_tensor = std::make_unique<mat3_t>(
                sdf->get_uniform_inertia_tensor(get_mass())
            );
        } else {
            throw std::runtime_error("Error: non uniform substances not yet supported.");
        }

        *inverse_inertia_tensor = mat::inverse(*inverse_inertia_tensor);
    }
    
    return *inverse_inertia_tensor;
}

vec3_t
matter_t::get_velocity(const vec3_t & x){
    return v + vec::cross(omega, get_offset_from_centre_of_mass(x));
}

vec3_t
matter_t::get_offset_from_centre_of_mass(const vec3_t & x){
    return x - transform.to_global_space(get_centre_of_mass()); 
}

vec3_t
matter_t::to_local_space(const vec3_t & x) const {
    return transform.to_local_space(x);
}

void
matter_t::reset_velocity(){
    v = vec3_t();
    omega = vec3_t();
    
    a = vec3_t();
    alpha = vec3_t();
}
