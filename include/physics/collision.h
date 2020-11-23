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
        vec3_t n_a;
        vec3_t n_b;

        double depth;
        std::shared_ptr<matter_t> a;
        std::shared_ptr<matter_t> b;

        collision_t(
            bool hit, const vec3_t & x, double fx,
            std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
        );
        
        void correct();
        void resting_correct();
    };

    collision_t collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b);
    void colliding_contact_correct(const collision_t & collision);
}



#endif
