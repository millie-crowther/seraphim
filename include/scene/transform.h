#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <mutex>
#include <vector>

#include "maths/quat.h"

class transform_t {
private:
    vec3_t position;
    std::mutex position_mutex;

    quat_t rotation;
    std::mutex rotation_mutex;

public:
    void set_position(const vec3_t & x);
    void translate(const vec3_t & x);

    void set_rotation(const quat_t & q);
    void rotate(const quat_t & q);

    // accessors
    vec3_t get_position() const;
    quat_t get_rotation() const;

    vec3_t to_local_space(const vec3_t & x) const;
};

#endif
