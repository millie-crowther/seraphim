#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <mutex>
#include <vector>

#include "maths/quat.h"
#include "maths/vector.h"

typedef struct srph_transform {
    srph::vec3_t position;
    srph::quat_t rotation;
    std::shared_ptr<srph::f32mat4_t> matrix;
    
    void recalculate_matrix();

    srph::vec3_t get_position() const;
    void set_position(const srph::vec3_t & x);
    void translate(const srph::vec3_t & x);

    srph::quat_t get_rotation() const;
    void rotate(const srph::quat_t & q);

    // accessors
    srph::vec3_t forward() const;
    srph::vec3_t up() const;
    srph::vec3_t right() const;

    srph::vec3_t to_local_direction(const srph::vec3_t & x) const;
    srph::vec3_t to_local_space(const srph::vec3_t & x) const;
    srph::vec3_t to_global_space(const srph::vec3_t & x) const;

    srph::f32mat4_t get_matrix();
} srph_transform;

void srph_transform_to_local_space (srph_transform * tf, vec3 * tx, const vec3 * x);
void srph_transform_to_global_space(srph_transform * tf, vec3 * tx, const vec3 * x);

#endif
