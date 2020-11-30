#ifndef MATTER_H
#define MATTER_H

#include "maths/sdf/sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "physics/transform.h"

#include <memory>

namespace srph {
    class matter_t {
    public:
        matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform);

        material_t get_material(const vec3_t & x);
        std::shared_ptr<sdf3_t> get_sdf() const;
        vec3_t get_position() const;
        double get_mass();
        aabb3_t get_aabb() const;

        void translate(const vec3_t & x);
        void rotate(const quat_t & q);

        quat_t get_rotation() const;

        bool is_inert(double delta);

        vec3_t get_velocity(const vec3_t & x);

        vec3_t get_acceleration(const vec3_t & x);
        void constrain_acceleration(const vec3_t & da);
        
        vec3_t get_offset_from_centre_of_mass(const vec3_t & x);
        vec3_t to_local_space(const vec3_t & x) const;

        void physics_tick(double delta);

        double get_inverse_angular_mass(const vec3_t & x, const vec3_t & n);
        
        void apply_impulse(const vec3_t & j);
        void apply_impulse_at(const vec3_t & j, const vec3_t & x);

        f32mat4_t * get_matrix();

        void apply_force(const vec3_t & f);
        void apply_force_at(const vec3_t & f, const vec3_t & x);

    private:
        material_t material;
        std::shared_ptr<sdf3_t> sdf;
        transform_t transform;
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

        void calculate_centre_of_mass();
        double get_average_density();
        vec3_t get_centre_of_mass();

        mat3_t * get_i();
        mat3_t * get_inv_tf_i();
    };
}

#endif
