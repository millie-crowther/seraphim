#ifndef MATTER_H
#define MATTER_H

#include "sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "scene/transform.h"

#include <memory>

class matter_t {
public:
    matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material);

    material_t get_material(const vec3_t & x);

    std::shared_ptr<sdf3_t> get_sdf() const;

    vec3_t get_position() const;
    void set_position(const vec3_t & x);

    void set_rotation(const quat_t & q);

    double phi(const vec3_t & x) const;

    double get_mass();

    transform_t & get_transform();


private:
    material_t material;
    std::shared_ptr<sdf3_t> sdf;

    transform_t transform;

    double get_average_density();
};

#endif