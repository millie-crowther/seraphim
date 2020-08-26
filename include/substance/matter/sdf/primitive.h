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
        n_sphere_t(double r){
            this->r = r;
        }

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
    };  

    typedef n_sphere_t<2> circle_t;
    typedef n_sphere_t<3> sphere_t;

    template<uint8_t D>
    class n_box_t : public sdf_t<D> {
    private:
        vec_t<double, D> r;

    public:
        n_box_t(const vec_t<double, D> & r){
            this->r = r;
        }

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
            return vec::volume(r * 2);
        }
    };

    typedef n_box_t<2> rectangle_t;
    typedef n_box_t<3> cuboid_t;
}

#endif
