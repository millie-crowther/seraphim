#include "metaphysics/matter.h"

using namespace srph;

matter_t::matter_t(srph_sdf * sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform){
    this->sdf = sdf;
    this->material = material;
    this->is_uniform = is_uniform;
    
    transform.set_position(initial_position);
    previous_position = initial_position;
}

quat_t matter_t::get_rotation() const {
    return transform.get_rotation();
}

vec3_t matter_t::get_position() const {
    return transform.get_position();
}

bool matter_t::is_inert(){
//    std::cout << "-----" << std::endl;
//    std::cout << "\tv = " << vec::length(v) << std::endl;
//    std::cout << "\to = " << vec::length(omega) << std::endl;


//    if (std::max(vec::length(v), vec::length(omega)) < constant::epsilon){
//        std::cout << "beep" << std::endl;
//    }

    return 
        vec::length(v) < constant::epsilon &&
        vec::length(omega) < constant::epsilon &&
        vec::length(a) < constant::epsilon &&
        vec::length(alpha) < constant::epsilon;
}

material_t matter_t::get_material(const vec3_t & x){
    return material;
}

bound3_t matter_t::get_bound() const {
    bound3_t bound;

    bound3_t * sdf_bound = srph_sdf_bound(sdf);
    for (int i = 0; i < 8; i++){  
        vec3_t x = sdf_bound->get_lower();
        for (int j = 0; j < 3; j++){
            if (i & (1 << j)){
                x[j] = sdf_bound->get_upper()[j];
            }
        }
        bound.capture(transform.to_global_space(x));
    }

    return bound;
}

bound3_t matter_t::get_moving_bound(double t) const {
    bound3_t bound = get_bound();
    
    bound += v * interval_t<double>(0, t);

    // TODO: include rotation    

    return bound;
}

double matter_t::get_inverse_angular_mass(const vec3_t & r_global, const vec3_t & n){
    auto r = r_global - transform.to_global_space(get_centre_of_mass()); 
    auto rn = vec::cross(r, n);

    return vec::dot(rn, *get_inv_tf_i() * rn);
}

bound3_t matter_t::velocity_bounds(const vec3_t & x, const interval_t<double> & t){
    bound3_t com = v * t + transform.to_global_space(get_centre_of_mass());
    return v + vec::cross(omega, x - com);
}

void matter_t::apply_impulse(const vec3_t & j){
    v += j / get_mass();
}

f32mat4_t * matter_t::get_matrix(){
    return transform.get_matrix();
}

void matter_t::apply_force(const vec3_t & f){
    a += f / get_mass();
}

void matter_t::apply_force_at(const vec3_t & f, const vec3_t & x){
    a += f / get_mass();

    auto r = x - transform.to_global_space(get_centre_of_mass()); 
    alpha += *get_inv_tf_i() * vec::cross(r, f);
}    

void matter_t::apply_impulse_at(const vec3_t & j, const vec3_t & r_global){
    v += j / get_mass();

    auto r = r_global - transform.to_global_space(get_centre_of_mass()); 
    omega += *get_inv_tf_i() * vec::cross(r, j);
}

void matter_t::calculate_centre_of_mass(){
    throw std::runtime_error("Error: autocalc of centre of mass not yet implemented.");
}

vec3_t matter_t::get_centre_of_mass(){
    if (is_uniform){
        vec3 c = srph_sdf_com(sdf);
        return vec3_t(c.x, c.y, c.z);
    }

    if (!centre_of_mass){
        calculate_centre_of_mass();
    }

    return *centre_of_mass;
}

void matter_t::constrain_acceleration(const vec3_t & da){
    a += da;
}

double matter_t::get_average_density(){
    if (is_uniform){
        return get_material(vec3_t()).density;
    }

    if (!average_density){
        calculate_centre_of_mass();
    }

    return *average_density;
}

double matter_t::get_mass(){
    return get_average_density() * srph_sdf_volume(sdf);
}

void matter_t::translate(const vec3_t & x){
    transform.translate(x);
}

void matter_t::rotate(const quat_t & q){
    transform.rotate(q);
    inv_tf_i.reset();
}

void matter_t::reset_acceleration() {
    a = vec3_t(0.0, -9.8, 0.0);
    alpha = vec3_t();
}

void matter_t::physics_tick(double t){
    // update position
    transform.translate((0.5 * a * t + v) * t);
    
    // update rotation
    transform.rotate(quat_t::euler_angles((0.5 * alpha * t + omega) * t));

    // integrate accelerations into velocities
    v += a * t;
    omega += alpha * t;
    
    if (transform.get_position()[1] < -90.0){
        transform.set_position(vec3_t(0.0, -100.0, 0.0));
        v = vec3_t();
        omega = vec3_t();
        a = vec3_t();
        alpha = vec3_t();
    }    
}

mat3_t * matter_t::get_inv_tf_i(){
    if (!inv_tf_i){
        inv_tf_i = std::make_unique<mat3_t>(*get_i());

        // rotate
        auto r = transform.get_rotation().to_matrix();
        *inv_tf_i = r * *inv_tf_i * mat::transpose(r);
        
        // invert
        *inv_tf_i = mat::inverse(*inv_tf_i);
    }

    return inv_tf_i.get();
}

mat3_t * matter_t::get_i(){
    if (!i){
        if (is_uniform){
            i = std::make_unique<mat3_t>(
                srph_sdf_inertia_tensor(sdf) * get_mass()
            );
        } else {
            throw std::runtime_error("Error: non uniform substances not yet supported.");
        }
    }
    
    return i.get();
}

vec3_t matter_t::get_velocity(const vec3_t & x){
    vec3_t x1 = x - transform.to_global_space(get_centre_of_mass());
    return v + vec::cross(omega, x1);
}

vec3_t matter_t::to_local_space(const vec3_t & x) const {
    return transform.to_local_space(x);
}
