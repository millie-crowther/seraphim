#include "aabb.h"

#include "maths/maths.h"

aabb_t
aabb_t::get_octant(int octant) const {
    // TODO
    return aabb_t();
}

vec3_t
aabb_t::get_centre() const {
    return min + vec3_t(size / 2);
}

double
aabb_t::get_upper_radius() const {
    return maths::inverse_square_root(0.75 * size * size);
}

double 
aabb_t::get_size() const { 
    return size;
}

bool
aabb_t::contains(const vec3_t & x) const {
    // TODO: unroll loop
    for (int i = 0; i < 3; i++){
        if (x[i] < min[i] || x[i] > min[i] + size){
            return false;
        }
    }
    return true;
}