#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <memory>

#include "sdf.h"

namespace srph { namespace primitive {
    template<uint8_t D>
    class n_sphere_t : public sdf_t<D> {
    private:
        double r;

    public:
        n_sphere_t(double _r) : r(_r) {}

        double phi(const vec_t<double, D> & x) override {
            return vec::length(x) - r;
        }

        vec_t<double, D> normal(const vec_t<double, D> & x) override {
            return vec::normalise(x);
        }

        interval_t<vec_t<double, D>> get_interval() override {
            return interval_t<vec_t<double, D>>(-r, vec_t<double, D>(r));
        }

        double get_volume() override {
            constexpr double coeff = std::pow(constant::pi, double(D) / 2.0) / std::tgamma(double(D) / 2.0 + 1.0);
            return coeff * std::pow(r, D);
        }

        mat3_t get_uniform_inertia_tensor(double mass) override {
            double i = 0.4 * mass * std::pow(r, 2);
            return mat3_t::diagonal(i);
        }
    };  

    typedef n_sphere_t<2> circle_t;
    typedef n_sphere_t<3> sphere_t;

    class torus_t : public sdf3_t {
    private:
        double r1;
        double r2;
    
    public:
        torus_t(double _r1, double _r2) : r1(_r1), r2(_r2) {}

        double phi(const vec3_t & x) override {
           vec2_t q = vec2_t(vec::length(vec2_t(x[0], x[2])) - r1, x[1]);
           return vec::length(q) - r2;     
        }

        interval_t<vec3_t> get_interval() override {
            vec3_t r(r1 + r2, r2, r1 + r2);
            return interval_t<vec3_t>(-r, r);
        }

        double get_volume() override {
            return 2.0 * constant::pi * constant::pi * r1 * r2 * r2;   
        }

        mat3_t get_uniform_inertia_tensor(double mass) override {
            double xz = 1.0 / 8.0 * mass * (4.0 * r1 * r1 + 3.0 * r2 * r2);
            double y  = 1.0 / 4.0 * mass * (4.0 * r1 * r1 + 5.0 * r2 * r2);
            return mat3_t(
                xz, 0, 0,
                0,  y, 0,
                0,  0, xz
            );
        }
    }; 
}}

#endif
