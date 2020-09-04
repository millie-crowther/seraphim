#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <mutex>
#include <vector>

#include "maths/quat.h"

class transform_t {
private:
    vec3_t position;

    quat_t rotation;

    std::unique_ptr<f32mat4_t> matrix;
    void recalculate_matrix();

public:
    transform_t();

    vec3_t get_position() const;
    void set_position(const vec3_t & x);
    void translate(const vec3_t & x);

    quat_t get_rotation() const;
    void rotate(const quat_t & q);

    // accessors
    vec3_t forward() const;
    vec3_t up() const;
    vec3_t right() const;

    vec3_t to_local_space(const vec3_t & x);

    f32mat4_t * get_matrix();
};

#endif
