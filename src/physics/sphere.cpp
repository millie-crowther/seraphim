#include "physics/sphere.h"

bool srph_sphere_intersect(const srph_sphere * a, const srph_sphere * b){
    vec3 d;
    srph_vec3_subtract(&d, &a->c, &b->c);
    return srph_vec3_length(&d) <= a->r + b->r;
}

bool srph_sphere_contains(const srph_sphere * s, const vec3 * x){
    vec3 d;
    srph_vec3_subtract(&d, &s->c, x);
    return srph_vec3_length(&d) <= s->r;
}
