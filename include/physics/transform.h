#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <mutex>
#include <vector>

#include "maths/quat.h"

namespace srph {
    class transform_t {
    private:
        vec3_t position;
        quat_t rotation;
        std::shared_ptr<f32mat4_t> matrix;
        
        void recalculate_matrix();

    public:
        vec3_t get_position() const;
        void set_position(const vec3_t & x);
        void translate(const vec3_t & x);

        quat_t get_rotation() const;
        void rotate(const quat_t & q);

        // accessors
        vec3_t forward() const;
        vec3_t up() const;
        vec3_t right() const;

        vec3_t to_local_direction(const vec3_t & x) const;
        vec3_t to_local_space(const vec3_t & x) const;
        vec3_t to_global_space(const vec3_t & x) const;

        f32mat4_t * get_matrix();
    };
}

#endif
