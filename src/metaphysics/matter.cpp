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

bound3_t matter_t::get_bound() const {
    bound3_t bound;
    
    for (int i = 0; i < 8; i++){  
        vec3_t x = sdf->get_bound().get_lower();
        for (int j = 0; j < 3; j++){
            if (i & (1 << j)){
                x[j] = sdf->get_bound().get_upper()[j];
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

bound3_t matter_t::velocity_bounds(const bound3_t & x, const interval_t<double> & t){
    transform_t tf = get_transform_after(t.get_lower());
    bound3_t com = v * t + tf.to_global_space(get_centre_of_mass());
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

bound3_t matter_t::normal_bounds(const bound3_t & b){
    bound3_t local_bounds;
    
    for (int i = 0; i < 8; i++){
        local_bounds.capture(transform.to_local_space(b.vertex(i)));    
    }

    bound3_t normal_bounds = sdf->get_normal_bounds(local_bounds);

    return transform.get_rotation() * normal_bounds; 
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

bool matter_t::is_locally_planar(const bound3_t & b) const {
    vec3_t centre = to_local_space(b.get_midpoint());
    vec3_t n = sdf->normal(centre);
    double d = vec::dot(centre, n) - sdf->phi(centre);

    // check each vertex maps onto the same plane
    for (int i = 0; i < 8; i++){
        vec3_t vertex = to_local_space(b.vertex(i));
        
        double di = vec::dot(vertex, n) - sdf->phi(vertex);
        if (std::abs(d - di) > constant::epsilon){
            return false;
        }

        if (sdf->normal(vertex) != n){
            return false;
        }
    }

    // TODO:
    // this is not a perfect detector of a planar region. However,
    // it might be good enough! If it proves to be insufficient somehow,
    // further restriction will be necessary below.

    return true; 
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

vec3_t matter_t::get_velocity(const vec3_t & x){
    vec3_t x1 = x - transform.to_global_space(get_centre_of_mass());
    return v + vec::cross(omega, x1);
}

vec3_t matter_t::to_local_space(const vec3_t & x) const {
    return transform.to_local_space(x);
}
