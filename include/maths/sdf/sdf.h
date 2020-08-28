#ifndef SDF_H
#define SDF_H

#include <functional>
#include "core/constant.h"
#include "maths/aabb.h"

template<uint8_t D>
class sdf_t {
protected:
    // protected constructor for abstract class
    sdf_t(){}

public:
    // virtual accessors 
    virtual double phi(const vec_t<double, D> & x) = 0;

    virtual vec_t<double, D> normal(const vec_t<double, D> & x) {
        auto f = std::bind(&sdf_t<D>::phi, this, std::placeholders::_1);
        return vec::grad(f, x);
    }

    virtual bool contains(const vec_t<double, D> & x){
        return phi(x) <= 0.0;
    }

    virtual aabb_t<double, D> get_aabb(){
        return aabb_t<double, D>(vec_t<double, D>(-constant::rho), vec_t<double, D>(constant::rho));
    }

    virtual double get_volume(){
        return 0.0;
    }

    virtual mat3_t get_uniform_inertia_tensor(double mass){
        // TODO
        return mat3_t::identity(); 
    }

    virtual vec3_t get_uniform_centre_of_mass(){
        return vec3_t();
    }
};

typedef sdf_t<3> sdf3_t;

#endif
