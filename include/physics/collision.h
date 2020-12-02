#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    class collision_t {
    private:
        bool intersecting;
        bool anticipated;
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
            double delta_t,
            std::shared_ptr<matter_t> a, std::shared_ptr<matter_t> b
        );
       
        bool is_intersecting() const;
        bool is_anticipated() const;
        double get_estimated_time() const;

        void correct();
        void resting_correct();
        void colliding_correct();

        struct comparator_t {
            bool operator()(const collision_t & a, const collision_t & b);
        };
    };
}



#endif
