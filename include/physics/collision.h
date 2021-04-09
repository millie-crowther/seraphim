#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "maths/vector.h"
#include "metaphysics/substance.h"

typedef struct srph_collision {
    srph_matter * ms[2];
    vec3 x;
} srph_collision;

bool srph_collision_is_detected(srph_collision * c, srph_substance * a, srph_substance * b, double dt);
void srph_collision_resolve_interpenetration_constraint(srph_collision * c);
void srph_collision_correct(srph_collision *self, double dt);

#endif
