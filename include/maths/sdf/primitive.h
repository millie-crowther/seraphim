#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <memory>

#include "sdf.h"

namespace primitive {
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

        aabb_t<double, D> get_aabb() override {
            return aabb_t<double, D>(-r, vec_t<double, D>(r));
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
}

#endif
