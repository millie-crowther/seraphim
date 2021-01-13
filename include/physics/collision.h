#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    class collision_t {
    private:
        static constexpr double density = constant::epsilon * 2;

        bool intersecting;
        bool anticipated;
        double t;

        vec3_t x;
        vec3_t x_a;
        vec3_t x_b;

        vec3_t n;
        vec3_t vr;

        double depth;
        matter_t * a;
        matter_t * b;

        void minimise(const aabb4_t & region);
        double lower_bound_t(const aabb4_t & region) const;
        double upper_bound_t(const aabb4_t & region) const;
        bool may_contain_collision(const aabb4_t & region) const;
        bool should_accept_region(const aabb4_t & region) const;
        std::pair<aabb4_t, aabb4_t> subdivide(const aabb4_t & region) const;        

    public:
        collision_t(double delta_t, matter_t * a, matter_t * b);
       
        bool is_intersecting() const;
        bool is_anticipated() const;
        double get_estimated_time() const;
        vec3_t get_position() const;
        std::pair<matter_t *, matter_t *> get_matters() const;

        void correct(const vec3_t & adjusted_x);
        void resting_correct();
        void colliding_correct();

        struct comparator_t {
            bool operator()(const collision_t & a, const collision_t & b);
        };
    };
}



#endif
