#ifndef SERAPHIM_PLATONIC_H
#define SERAPHIM_PLATONIC_H

#include "sdf.h"

typedef struct srph_sdf_cuboid {
    srph_sdf sdf;
    vec3 _r;
} srph_sdf_cuboid;

typedef struct srph_sdf_octahedron {
    srph_sdf sdf;
    double _e;
} srph_sdf_octahedron;

void srph_sdf_cuboid_create(srph_sdf_cuboid * c, const vec3 * r);
void srph_sdf_octahedron_create(srph_sdf_octahedron * o, double e);

#endif
