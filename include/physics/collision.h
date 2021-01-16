#ifndef COLLISION_H
#define COLLISION_H

#include "maths/matrix.h"
#include "metaphysics/matter.h"

namespace srph {
    class collision_t {
    private:
        struct region_t {
            aabb4_t region;

            region_t(){}
            region_t(const vec4_t & min, const vec4_t & max) : region(min, max){}            

            struct comparator_t {
                bool operator()(const region_t & a, const region_t & b) const;
            };
        };

        static constexpr double solution_density = constant::epsilon * 10;

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

        void minimise(const region_t & region);
        double lower_bound_t(const region_t & region) const;
        double upper_bound_t(const region_t & region) const;
        
        bool should_accept_solution(const region_t & region) const;
        bool contains_unique_solution(const region_t & region) const;
        std::pair<region_t, region_t> subdivide(const region_t & region) const;        
        
        bool satisfies_constraints(
            const region_t & region, double upper_t, const std::vector<region_t> & sing_solns
        ) const;

    public:
        collision_t(matter_t * a, matter_t * b);
       
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
