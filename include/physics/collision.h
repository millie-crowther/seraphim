#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace seraph { namespace physics {
    struct collision_t {
        bool hit;
        vec3_t x;
        double t;
        double fx;
        std::shared_ptr<matter_t> a;
        std::shared_ptr<matter_t> b;

        collision_t(
            bool hit, const vec3_t & x, double fx, 
            std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
        );

        struct comparator_t {
            bool operator()(const collision_t & a, const collision_t & b);
        };

        static collision_t null();
    };

    collision_t collide(std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b);
    void collision_correct(const collision_t & collision);
}}



#endif
