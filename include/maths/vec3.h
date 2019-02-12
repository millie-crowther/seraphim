#ifndef VEC3_H
#define VEC3_H

#include "vec.h"

class vec3_t : public vec_t<3> {
public:
    vec3_t cross(const vec3_t & v) const;
    vec3_t operator%(const vec3_t & v) const;
};

#endif