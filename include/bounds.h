#ifndef BOUNDS_H
#define BOUNDS_H

#include "vec.h"

class bounds_t {
private:
    vec3_t min;
    vec3_t size;

public:
    bounds_t(const vec3_t& m, const vec3_t& s);

    vec3_t get_centre() const;
    bool contains(const vec3_t& v);
    bounds_t get_octant(int i);
    int get_octant_id(const vec3_t& v);
};

#endif
