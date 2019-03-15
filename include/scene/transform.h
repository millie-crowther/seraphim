#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <vector>

#include "maths/quat.h"

class transform_t {
private:
    // private fields
    transform_t * master;
    std::vector<std::shared_ptr<transform_t>> servants;

    vec3_t position;
    quat_t rotation;
    
    // private constructor
    transform_t(transform_t * master);

public:
    // constructors and destructors
    transform_t();

    std::shared_ptr<transform_t> create_servant();
    
    vec3_t get_absolute_position() const;

    void set_position(const vec3_t & x);
    void set_rotation(const quat_t & q);
};

#endif
