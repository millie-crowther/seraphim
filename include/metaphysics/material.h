#ifndef SERAPHIM_MATERIAL_H
#define SERAPHIM_MATERIAL_H

#include "maths/maths.h"

typedef struct material_t {
    vec3 colour;
    double density;
    double restitution;
    double static_friction;
    double dynamic_friction;
} material_t;

#endif
