#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "maths/matrix.h"
#include "maths/vector.h"
#include "metaphysics/matter.h"

typedef struct srph_collision {
    bool is_intersecting;
    double t;

    vec3 x;
    vec3 xa;
    vec3 xb;

    srph_array xs;

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

#endif
