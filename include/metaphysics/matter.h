#ifndef MATTER_H
#define MATTER_H

#include "maths/sdf/sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "physics/transform.h"

#include <memory>

namespace srph {
    struct matter_t {
        transform_t transform;
    
        material_t material;
        srph_sdf * sdf;

        vec3_t previous_position;
        bool is_uniform;

        std::unique_ptr<double> average_density;
        std::unique_ptr<vec3_t> centre_of_mass;
        
        std::unique_ptr<mat3_t> i;
        std::unique_ptr<mat3_t> inv_tf_i;

        vec3_t v;
        vec3_t a;

        vec3_t omega;
        vec3_t alpha;

        matter_t(srph_sdf * sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform);

        material_t get_material(const vec3_t & x);
        srph_sdf * get_sdf() const;
        vec3_t get_position() const;
        double get_mass();

        bound3_t get_bound() const;
        bound3_t get_moving_bound(double t) const;

        void translate(const vec3_t & x);
        void rotate(const quat_t & q);

        quat_t get_rotation() const;

        bool is_inert();

        void constrain_acceleration(const vec3_t & da);
        void reset_acceleration();
        
        vec3_t to_local_space(const vec3_t & x) const;

        void physics_tick(double delta);
        
        vec3_t get_velocity(const vec3_t & x);

        double get_inverse_angular_mass(const vec3_t & x, const vec3_t & n);
        
        void apply_impulse(const vec3_t & j);
        void apply_impulse_at(const vec3_t & j, const vec3_t & x);

        f32mat4_t * get_matrix();

        void apply_force(const vec3_t & f);
        void apply_force_at(const vec3_t & f, const vec3_t & x);

        bound3_t velocity_bounds(const vec3_t & x, const interval_t<double> & t);

        void calculate_centre_of_mass();
        double get_average_density();
        vec3_t get_centre_of_mass();

        mat3_t * get_i();
        mat3_t * get_inv_tf_i();
    };
}

#endif
