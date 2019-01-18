#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "mat.h"

class transform_t {
private:
    // private fields
    transform_t * parent;

    mat4_t matrix;

public:
    // constructors and destructors
    transform_t();
    transform_t(transform_t & parent);

    // accessors
    mat4_t get_matrix() const;

    // modifiers
    void set_parent(transform_t & parent);
};

#endif
