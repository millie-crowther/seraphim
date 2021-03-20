#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "maths/matrix.h"
#include "maths/vector.h"
#include "metaphysics/substance.h"

typedef struct srph_collision {
    bool is_intersecting;
    double t;

    vec3 x;
    vec3 xa;
    vec3 xb;

    srph_array(vec3) xs;

    srph::vec3_t n;
    srph::vec3_t vr;

    double depth;
    srph_matter * a;
    srph_matter * b;

    srph_collision(srph_matter * a, srph_matter * b);

    void correct();
    void colliding_correct();

    struct comparator_t {
        bool operator()(const srph_collision & a, const srph_collision & b);
    };
} srph_collision;

bool srph_collision_is_detected(srph_substance * a, srph_substance * b, double dt);
void srph_collision_push_constraints(srph_constraint_array * cs, srph_substance * a, srph_substance * b);

#endif
