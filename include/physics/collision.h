#ifndef SERAPHIM_COLLISION_H
#define SERAPHIM_COLLISION_H

#include "maths/matrix.h"
#include "maths/vector.h"
#include "metaphysics/matter.h"

namespace srph {
    struct collision_t {
        static constexpr double solution_density = 1.0 / 5.0;

        bool intersecting;
        double t;

        vec3 x;
        vec3 xa;
        vec3 xb;

        vec3_t n;
        vec3_t vr;

        double depth;
        srph_matter * a;
        srph_matter * b;

        collision_t(srph_matter * a, srph_matter * b);
       
        bool is_intersecting() const;
        double get_estimated_time() const;

        void correct();
        void colliding_correct();

        struct comparator_t {
            bool operator()(const collision_t & a, const collision_t & b);
        };
    };
}

#endif
