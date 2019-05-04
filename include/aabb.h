#ifndef AABB_H
#define AABB_H

#include "maths/vec.h"

class aabb_t {
private:
    vec3_t min;
    double size;
public:

    // accessors
    aabb_t get_octant(int octant) const;
    vec3_t get_centre() const;
    double get_upper_radius() const;
    double get_size() const;
};

#endif