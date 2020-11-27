#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    struct collision_t {
        bool hit;

        vec3_t x;
        vec3_t x_a;
        vec3_t x_b;

        vec3_t n;
        vec3_t vr;

        double depth;
        std::shared_ptr<matter_t> a;
        std::shared_ptr<matter_t> b;

        collision_t(
            std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
        );
        
        void correct();
        void resting_correct();
        void colliding_correct();
    };
}



#endif
