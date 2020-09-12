#include "metaphysics/matter/matter.h"

matter_t::matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform){
    this->sdf = sdf;
    this->material = material;
    this->transform.set_position(initial_position);
    this->is_uniform = is_uniform;
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

void 
matter_t::calculate_centre_of_mass(){/*
    double total_density;
    vec3_t total_mass;

    for (uint32_t sample = 0; sample < number_of_samples;){
        vec3_t x = sdf->get_aabb().random();
        if (sdf->contains(x)){
            double density = get_material(x).density;
            total_density += density;
            total_mass += x * density;
            sample++;
        }
    }

    average_density = std::make_unique<double>(total_density / number_of_samples);
    centre_of_mass = std::make_unique<vec3_t>(total_mass / total_density);*/
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
matter_t::apply_force(const vec3_t & force){
    a += force / get_mass();
}

void
matter_t::physics_tick(double t){
    transform = get_transform_at(t);

    // integrate accelerations into velocities
    v += a * t;
    omega += alpha * t;
    
    // reset accelerations
    a = vec3_t(0.0, -9.8, 0.0);
    alpha = vec3_t(0.0);
}

transform_t
matter_t::get_transform_at(double t){
    transform_t tf = transform;
    
    // update position
    tf.translate(a * 0.5 * t * t + v * t);
    
    // update rotation
    auto w = alpha * 0.5 * t * t + omega * t;
    auto q = quat_t::angle_axis(vec::length(w), vec::normalise(w));
    tf.rotate(q);

    return tf;
}

mat3_t 
matter_t::get_inertia_tensor(){
    if (!inertia_tensor){
/*
        vec3_t com = get_centre_of_mass();
        
        double Ixx, Iyy, Izz, Ixy, Iyz, Ixz;

        for (uint32_t sample = 0; sample < number_of_samples;){
            vec3_t x = sdf->get_aabb().random();
            if (sdf->contains(x)){
                double density = get_material(x).density;
                
                vec3_t r = x - com;
                
                Ixx += density * (r[1] * r[1] + r[2] * r[2]);
                Iyy += density * (r[0] * r[0] + r[2] * r[2]);
                Izz += density * (r[1] * r[1] + r[2] * r[2]);

                Ixy -= density * r[0] * r[1];
                Ixz -= density * r[0] * r[2];
                Iyz -= density * r[1] * r[2];                
                
                sample++;
            }
        }
        
               
        mat3_t I(
            Ixx, Ixy, Ixz,
            Ixy, Iyy, Iyz,
            Ixz, Ixy, Izz 
        ); 

        I *= sdf->get_volume() / number_of_samples;
*/
        if (is_uniform){
            inertia_tensor = std::make_unique<mat3_t>(
                sdf->get_uniform_inertia_tensor(get_mass())
            );
        } else {
            throw std::runtime_error("Error: non uniform substances not yet supported.");
        }
    }

    auto i = *inertia_tensor;
    auto r = transform.get_rotation().to_matrix();
    i = r * i * mat::transpose(r);

    return i;
}

vec3_t
matter_t::get_velocity(const vec3_t & x){
    return v + omega * get_offset_from_centre_of_mass(x);
}

vec3_t
matter_t::get_offset_from_centre_of_mass(const vec3_t & x){
    return x - (transform.get_position() + get_centre_of_mass()); 
}

void
matter_t::update_velocities(const vec3_t & dv, const vec3_t & dw){
    v += dv;
    omega += dw;
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
