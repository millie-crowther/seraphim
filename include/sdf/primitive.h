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
    class cuboid_t : public sdf_t<D> {
    private:
        vec_t<double, D> c;
        vec_t<double, D> r;

    public:
        cuboid_t(const vec_t<double, D> & c, const vec_t<double, D> & r){
            this->c = c;
            this->r = r;
        }

        double phi(const vec_t<double, D> & x) const override {
            vec_t<double, D> q = (x - c).abs() - r;
            return 
                q.max(vec_t<double, D>()).norm() +
                std::min(std::max(q[0], std::max(q[1], q[2])), 0.0);
        }
    };
}

#endif