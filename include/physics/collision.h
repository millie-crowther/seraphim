#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    class collision_t {
    private:
        static constexpr double solution_density = 1.0 / 4.0;

        bool intersecting;
        bool anticipated;
        double t;

        vec3_t x;
        vec3_t x_a;
        vec3_t x_b;

        vec3_t n;
        vec3_t vr;

        std::vector<vec3_t> min_xs;
        double min_t;

        double depth;
        matter_t * a;
        matter_t * b;

        void minimise(const bound4_t & bound);

        double time_to_collision(const bound3_t & b);
        
        bool should_accept_solution(const bound4_t & bound) const;
        bool contains_unique_solution(const bound4_t & bound) const;
        std::pair<bound4_t, bound4_t> subdivide(const bound4_t & bound) const;        
        
        bool satisfies_constraints(
            const bound4_t & region, double upper_t, const std::vector<bound4_t> & sing_solns
        ) const;

    public:
        collision_t(matter_t * a, matter_t * b);
       
        bool is_intersecting() const;
        bool is_anticipated() const;
        double get_estimated_time() const;
        vec3_t get_position() const;
        std::pair<matter_t *, matter_t *> get_matters() const;

        void correct();
        void resting_correct();
        void colliding_correct();

        struct comparator_t {
            bool operator()(const collision_t & a, const collision_t & b);
        };
    };
}



#endif
