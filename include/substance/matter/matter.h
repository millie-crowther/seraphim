#ifndef MATTER_H
#define MATTER_H

#include "sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "scene/transform.h"

#include <memory>

class matter_t {
public:
    matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material, const vec3_t & initial_position);

    material_t get_material(const vec3_t & x);
    std::shared_ptr<sdf3_t> get_sdf() const;
    vec3_t get_position() const;
    double get_mass();
    transform_t & get_transform();

    double phi(const vec3_t & x) const;

    void apply_force(const vec3_t & force);
    void physics_tick(double delta);

private:
    static constexpr uint32_t number_of_samples = 10000;

    material_t material;
    std::shared_ptr<sdf3_t> sdf;
    transform_t transform;

    std::unique_ptr<double> average_density;
    std::unique_ptr<vec3_t> centre_of_mass;

    vec3_t velocity;
    vec3_t acceleration;

    void calculate_centre_of_mass();
    double get_average_density();
    vec3_t get_centre_of_mass();
};

#endif