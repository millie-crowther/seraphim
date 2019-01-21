#ifndef RAY_H
#define RAY_H

#include "maths/vec.h"
#include "physics/collider.h"

class ray_t {
private:
    vec3_t origin;
    vec3_t direction;

public:
    class intersection_t {
        
    };

    ray_t(const vec3_t& o, const vec3_t& d);

    intersection_t cast(const std::vector<collider_t>& colliders);
};

#endif