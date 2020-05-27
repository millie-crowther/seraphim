#ifndef SDF_H
#define SDF_H

#include "maths/vec.h"
#include "maths/aabb.h"
#include <functional>
#include "core/hyper.h"

template<uint8_t D>
class sdf_t {
protected:
    // protected constructor for abstract class
    sdf_t(){}

public:
    virtual ~sdf_t(){}

    // virtual accessors 
    virtual double phi(const vec_t<double, D> & x) const = 0;

    virtual vec_t<double, D> normal(const vec_t<double, D> & x) const {
        auto f = std::bind(&sdf_t<D>::phi, this, std::placeholders::_1);
        return vec_t<double, D>::nabla(f, x, hyper::epsilon);
    }
};

typedef sdf_t<3> sdf3_t;

#endif