#include "metaphysics/matter.h"

using namespace srph;

matter_t::matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform){
    this->sdf = sdf;
    this->material = material;
    this->is_uniform = is_uniform;
    
    transform.set_position(initial_position);
    previous_position = initial_position;
        
    omega = vec3_t(0.01);
}

std::shared_ptr<sdf3_t> matter_t::get_sdf() const {
    return sdf;
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

interval_t<vec3_t> matter_t::get_moving_interval(double t) const {
    interval_t<vec3_t> interval;
    
    for (uint8_t i = 0; i < 8; i++){   
        vec3_t x = sdf->get_interval().get_min();
        for (int j = 0; j < 3; j++){
            if (i & (1 << j)){
                x[j] = sdf->get_interval().get_max()[j];
            }
        }
        interval.capture(transform.to_global_space(x));
    }

    interval.enlarge(0.5 * vec::length(a) * t * t + vec::length(v) * t);

    return interval;
}

double matter_t::get_inverse_angular_mass(const vec3_t & r_global, const vec3_t & n){
    auto r = r_global - transform.to_global_space(get_centre_of_mass()); 
    auto rn = vec::cross(r, n);

    return vec::dot(rn, *get_inv_tf_i() * rn);
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
        return sdf->get_uniform_centre_of_mass();
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
    return get_average_density() * sdf->get_volume();
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

transform_t matter_t::get_transform_after(double t){
    transform_t tf = transform;

    // update position
    tf.translate((0.5 * a * t + v) * t);
    
    // update rotation
    tf.rotate(quat_t::euler_angles((0.5 * alpha * t + omega) * t));

    return tf;
}

void matter_t::physics_tick(double t){
    transform = get_transform_after(t);

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
                sdf->get_uniform_inertia_tensor(get_mass())
            );
        } else {
            throw std::runtime_error("Error: non uniform substances not yet supported.");
        }
    }
    
    return i.get();
}

vec3_t matter_t::get_velocity_after(const vec3_t & x, double t){
    transform_t tf = get_transform_after(t);
    vec3_t x1 = x - tf.to_global_space(get_centre_of_mass());
    return v + vec::cross(omega, x1);
}

vec3_t matter_t::to_local_space(const vec3_t & x) const {
    return transform.to_local_space(x);
}
