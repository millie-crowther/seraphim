#ifndef SERAPHIM_MATTER_H
#define SERAPHIM_MATTER_H

#include "maths/sdf/sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "physics/sphere.h"
#include "physics/transform.h"

#include <memory>

typedef struct srph_matter {
    srph_transform transform;

    srph_material material;
    srph_sdf * sdf;

    srph::vec3_t previous_position;
    bool is_uniform;

    bool _is_mass_calculated;
    double average_density;
    srph::vec3_t centre_of_mass;    

    bool _is_inertia_tensor_valid;
    srph::mat3_t i;

    bool _is_inv_inertia_tensor_valid;
    srph::mat3_t inv_tf_i;

    srph::vec3_t v;
    srph::vec3_t a;

    srph::vec3_t omega;
    srph::vec3_t alpha;

    srph_matter(){}
    srph_matter(srph_sdf * sdf, const srph_material * material, const srph::vec3_t & initial_position, bool is_uniform);

    srph_material get_material(const vec3 * x);
    srph_sdf * get_sdf() const;
    srph::vec3_t get_position() const;
    double get_mass();

    srph_bound3 get_moving_bound(double t) const;

    void translate(const srph::vec3_t & x);
    void rotate(const srph::quat_t & q);

    srph::quat_t get_rotation() const;

    bool is_inert();
    
    srph::vec3_t to_local_space(const srph::vec3_t & x) const;

    void physics_tick(double delta);
    
    srph::vec3_t get_velocity(const srph::vec3_t & x);

    double get_inverse_angular_mass(const srph::vec3_t & x, const srph::vec3_t & n);
    
    void apply_impulse(const srph::vec3_t & j);
    void apply_impulse_at(const srph::vec3_t & j, const srph::vec3_t & x);

    srph::f32mat4_t * get_matrix();

    void apply_force(const srph::vec3_t & f);
    void apply_force_at(const srph::vec3_t & f, const srph::vec3_t & x);

    void reset_acceleration();

    void calculate_centre_of_mass();
    double get_average_density();
    srph::vec3_t get_centre_of_mass();

    srph::mat3_t * get_i();
    srph::mat3_t * get_inv_tf_i();
} srph_matter;

void srph_matter_bound(const srph_matter * m, srph_bound3 * b);
void srph_matter_sphere_bound(const srph_matter * m, double t, srph_sphere * s);

#endif
