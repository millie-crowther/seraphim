#include "core/aabb.h"

#include "sdf/primitive.h"

#include <math.h>

aabb_t::aabb_t(){}

aabb_t::aabb_t(const vec3_t & min, double size){
    this->min = min;
    this->size = size;
}

int
aabb_t::get_octant(const vec3_t & x) const {
    int result = 0;
    for (int i = 0; i < 3; i++){
        if (x[i] > min[i] + size / 2){
            result &= 1 << i;
        } 
    }
    return result;
}

void
aabb_t::refine(uint8_t octant){
    size /= 2;
    for (uint8_t i = 0; i < 3; i++){
        if (octant & (1 << i)){
            min[i] += size;
        }
    }
}

vec3_t
aabb_t::get_centre() const {
    return min + vec3_t(size / 2);
}

double
aabb_t::get_upper_radius() const {
    return sqrt(0.75 * size * size);
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