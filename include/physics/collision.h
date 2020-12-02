#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    class collision_t {
    private:
        bool is_intersecting;
        bool is_anticipated;
        double t;

        vec3_t x;
        vec3_t x_a;
        vec3_t x_b;

        vec3_t n;
        vec3_t vr;

        double depth;
        std::shared_ptr<matter_t> a;
        std::shared_ptr<matter_t> b;

    public:
        collision_t(
            double delta,
            std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
        );
       
        bool is_colliding() const;
 
        void correct();
        void resting_correct();
        void colliding_correct();
    };
}



#endif
