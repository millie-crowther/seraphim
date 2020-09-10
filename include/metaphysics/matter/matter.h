#ifndef MATTER_H
#define MATTER_H

#include "maths/sdf/sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "physics/transform.h"

#include <memory>

class matter_t {
public:
    matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position, bool is_uniform);

    material_t get_material(const vec3_t & x);
    std::shared_ptr<sdf3_t> get_sdf() const;
    vec3_t get_position() const;
    double get_mass();
    transform_t & get_transform();

    double phi(const vec3_t & x);
    vec3_t normal(const vec3_t & x);

    vec3_t get_velocity(const vec3_t & x);
    vec3_t get_offset_from_centre_of_mass(const vec3_t & x);
    mat3_t get_inertia_tensor();
    vec3_t to_local_space(const vec3_t & x) const;

    void apply_force(const vec3_t & force);
    void physics_tick(double delta);
    void update_velocities(const vec3_t & dv, const vec3_t & dw);
    void reset_velocity();

    transform_t get_transform_at(double t);

private:
    static constexpr uint32_t number_of_samples = 10000;

    material_t material;
    std::shared_ptr<sdf3_t> sdf;
    transform_t transform;
    bool is_uniform;

    std::unique_ptr<double> average_density;
    std::unique_ptr<vec3_t> centre_of_mass;
    std::unique_ptr<mat3_t> inertia_tensor;

    vec3_t v;
    vec3_t a;

    vec3_t omega;
    vec3_t alpha;

    void calculate_centre_of_mass();
    double get_average_density();
    vec3_t get_centre_of_mass();
};

#endif
