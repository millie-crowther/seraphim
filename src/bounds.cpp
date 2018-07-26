#include "bounds.h"

#include <limits>
#include "maths.h"

bounds_t::bounds_t() : bounds_t(vec3_t(), vec3_t()){ }

bounds_t::bounds_t(const vec3_t& m, const vec3_t& s){
    min = m;
    size = s;
}

bounds_t
bounds_t::get_octant(int i) const {
    vec3_t new_size = size / 2.0f;
    vec3_t new_min = min;

    if (i & 1) new_min[0] += new_size[0];
    if (i & 2) new_min[1] += new_size[1];
    if (i & 4) new_min[2] += new_size[2];

    return bounds_t(new_min, new_size);
}

vec3_t
bounds_t::get_centre() const {
    return min + size / 2.0f;
}

int
bounds_t::get_octant_id(const vec3_t& v) const {
    if (!contains(v)){
        return -1;
    }

    vec3_t centre = get_centre();

    int result = 0;
    if (v[0] > centre[0]) result |= 1;
    if (v[1] > centre[1]) result |= 2;
    if (v[2] > centre[2]) result |= 3;
    return result;
}

bool
bounds_t::contains(const vec3_t& v) const {
    return 
        min[0] <= v[0] && v[0] <= min[0] + size[0] &&
        min[1] <= v[1] && v[1] <= min[1] + size[1] &&
        min[2] <= v[2] && v[2] <= min[2] + size[2];
}

bounds_t
bounds_t::max_bounds(){
    // TODO
    vec3_t s(1 << 16);
    return bounds_t(-(s / 2.0f), s);
}

vec3_t
bounds_t::get_size() const {
    return size;
}

std::string
bounds_t::to_string() const {
    return "bounds[centre: " + get_centre().to_string() + ", size: " + size.to_string() + "]";
}

void
bounds_t::encapsulate_sphere(const vec3_t& v, float r){
    if (size.approx(vec3_t(0))){
	    min = v;
    }

    for (int i = 0; i < 3; i++){
        min[i] = std::min(min[i], v[i] - r);
        size[i] = std::max(size[i], v[i] + r - min[i]);
    }
}
