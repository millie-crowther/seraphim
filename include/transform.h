#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "mat.h"

class transform_t {
private:
    // private fields
    transform_t * parent;
    std::vector<transform_t *> children;
    mat4_t tf;

public:
    // constructors and destructors
    transform_t();

    // getters
    mat4_t get_tf_matrix();

    // setters
    void set_tf_matrix(const mat4_t& m);
};

#endif
