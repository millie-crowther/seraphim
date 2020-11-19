#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    struct collision_t {
        bool hit;
        vec3_t x;
        double t;
        double depth;
        std::shared_ptr<matter_t> a;
        std::shared_ptr<matter_t> b;

        collision_t(
            bool hit, const vec3_t & x, double fx,
            std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
        );
    };

    collision_t collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b);
    void collision_correct(const collision_t & collision);
    void resting_contact_correct(const collision_t & collision);
    void colliding_contact_correct(const collision_t & collision);
}



#endif
