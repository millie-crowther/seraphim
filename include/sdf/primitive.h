#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <memory>

#include "sdf/sdf.h"

// TODO: remove 'centre' param from all SDFs
//       translation operation should be performed by transform_t class

namespace primitive {
    template<uint8_t D>
    class sphere_t : public sdf_t<D> {
    private:
        vec_t<double, D> c;
        double r;
    public:
        sphere_t(const vec_t<double, D> & c, double r){
            this->c = c;
            this->r = r;
        }

        double phi(const vec_t<double, D> & x) const override {
            return (x - c).norm() - r;
        }

        vec_t<double, D> normal(const vec_t<double, D> & x) const override {
            return (x - c).normalise();
        }
    };  

    template<uint8_t D>
    class plane_t : public sdf_t<D> {
    private:
        vec_t<double, D> n;
        double d;
    public:
        plane_t(const vec_t<double, D> & n, double d){
            this->n = n;
            this->d = d;
        }

        double phi(const vec_t<double, D> & x) const override {
            return x * n - d;
        }

        vec_t<double, D> normal(const vec_t<double, D> & x) const override {
            return n;
        }
    };
}

#endif