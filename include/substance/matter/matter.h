#ifndef MATTER_H
#define MATTER_H

#include "sdf.h"
#include "material.h"
#include "maths/quat.h"
#include "scene/transform.h"

#include <memory>

class matter_t {
public:
    struct data_t {
        float near;
        float far;
        float _1;
        float _2;

        f32vec3_t r;
        uint32_t id;

        f32mat4_t transform;

        data_t();
        data_t(float near, float far, const f32vec3_t & r, uint32_t id, const f32mat4_t & transform);

        struct comparator_t {
            bool operator()(const matter_t::data_t & a, const matter_t::data_t & b) const;
        };
    };

    matter_t(std::shared_ptr<sdf3_t> sdf, const material_t & material);

    material_t get_material(const vec3_t & x);

    std::shared_ptr<sdf3_t> get_sdf() const;
    data_t get_data(const vec3_t & eye_position);
    uint32_t get_id() const;

    vec3_t get_position() const;
    void set_position(const vec3_t & x);

    void set_rotation(const quat_t & q);

    double phi(const vec3_t & x) const;

private:
    uint32_t id;
    material_t material;
    std::shared_ptr<sdf3_t> sdf;

    transform_t transform;
};

#endif