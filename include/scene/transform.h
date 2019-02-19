#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "maths/quat.h"

class transform_t {
private:
    // private fields
    transform_t * parent;

    vec3_t position;
    quat_t rotation;

public:
    // constructors and destructors
    transform_t();
    transform_t(transform_t & parent);

    // accessors
    mat4_t get_matrix() const;
    void set_matrix(const mat4_t & matrix);

    // modifiers
    void set_parent(transform_t & parent);
};

#endif
