#ifndef AABB_H
#define AABB_H

#include "maths/vec.h"

class aabb_t {
private:
    vec3_t min;
    double size;
public:
    // modifiers
    void refine(int octant);

    // accessors
    int get_octant(const vec3_t & x) const;
    vec3_t get_centre() const;
    double get_upper_radius() const;
    double get_size() const;
    bool contains(const vec3_t & x) const;
};

#endif
