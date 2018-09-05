#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "mat.h"

class transform_t {
private:
    transform_t * parent;
    std::vector<transform_t *> children;
    mat4_t tf;

public:
    transform_t();
};

#endif