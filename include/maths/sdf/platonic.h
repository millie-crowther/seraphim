#ifndef PLATONIC_H
#define PLATONIC_H

#include "sdf.h"

namespace platonic {
    template<uint8_t D>
    class n_cuboid_t : public sdf_t<D> {
    private:
        vec_t<double, D> r;
    
    public:
        n_cuboid_t(const vec_t<double, D> & _r) : r(_r) {}

        double phi(const vec_t<double, D> & x) override {
            auto q = mat::abs(x) - r;
            return 
                vec::length(mat::max(q, 0.0)) +
                std::min(*std::max_element(q.begin(), q.end()), 0.0);
        }

        aabb_t<double, D> get_aabb() override {
            return aabb_t<double, D>(-r, r);
        }

        double get_volume() override {
            return vec::volume(r) * std::pow(2, D);
        }

        mat3_t get_uniform_inertia_tensor(double mass) override {
            vec3_t r2 = r * r;
            mat3_t i(
                r2[1] + r2[2], 0.0,           0.0,
                0.0,           r2[0] + r2[2], 0.0,
                0.0,           0.0,           r2[0] + r2[1]
            );
           
            i *= mass / 12.0;
            return i;
        }
    };

    template<uint8_t D>
    class n_cube_t : public n_cuboid_t<D> {
    public:
        n_cube_t(double r) : n_cuboid_t<D>(r){}
    };
    
    typedef n_cuboid_t<2> rectangle_t;
    typedef n_cuboid_t<3> cuboid_t;
    
    typedef n_cube_t<2> square_t;    
    typedef n_cube_t<3> cube_t;
}

#endif
