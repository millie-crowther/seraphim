#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <vector>

#include "maths/quat.h"

class transform_t {
private:
    // private fields
    transform_t * parent;
    std::vector<std::shared_ptr<transform_t>> children;

    vec3_t position;
    quat_t rotation;
    
    // private constructor
    transform_t(transform_t * parent);

public:
    // constructors and destructors
    transform_t();

    std::shared_ptr<transform_t> add_child();
    
    vec3_t get_absolute_position() const;
};

#endif
