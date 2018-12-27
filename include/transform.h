#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "mat.h"

class transform_t {
private:
    // private fields
    transform_t * parent;
    // std::vector<transform_t *> children;
    mat4_t tf;

public:
    // constructors and destructors
    transform_t();

    // accessors
    mat4_t get_tf_matrix();

    // modifiers
    void set_tf_matrix(const mat4_t& m);
    void transform(const transform_t & tf);
};

#endif
