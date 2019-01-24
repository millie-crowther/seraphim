#ifndef RAY_H
#define RAY_H

#include "maths/vec.h"
#include "sdf.h"

class ray_t {
private:
    vec3_t origin;
    vec3_t direction;  

public:
    struct intersection_t {
        bool   is_hit;
        vec3_t position;
        vec3_t normal; 
    };

    ray_t(const vec3_t& o, const vec3_t& d);

    intersection_t cast(const sdf_t& sdf) const;
};

#endif