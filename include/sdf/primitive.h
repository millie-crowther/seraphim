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
        double r;
    public:
        sphere_t(double r){
            this->r = r;
        }

        double phi(const vec_t<double, D> & x) const override {
            return x.norm() - r;
        }

        vec_t<double, D> normal(const vec_t<double, D> & x) const override {
            return x.normalise();
        }
    };  

    template<uint8_t D>
    class cuboid_t : public sdf_t<D> {
    private:
        vec_t<double, D> r;

    public:
        cuboid_t(const vec_t<double, D> & r){
            this->r = r;
        }

        double phi(const vec_t<double, D> & x) const override {
            auto q = x.abs() - r;
            return 
                q.max(vec_t<double, D>()).norm() +
                std::min(*std::max_element(q.begin(), q.end()), 0.0);
        }
    };
}

#endif