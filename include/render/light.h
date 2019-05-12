#ifndef LIGHT_H
#define LIGHT_H

#include "maths/vec3.h"
#include "scene/transform.h"

#include <memory>

class light_t {
private:
    std::shared_ptr<transform_t> transform;
    
public:
    vec3_t get_position() const;
    double get_radius() const;
};

#endif
