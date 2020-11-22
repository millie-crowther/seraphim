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
        vec_t<double, D> n;
        for (int i = 0; i < D; i++){
            vec_t<double, D> axis;
            axis[i] = constant::epsilon;
            n[i] = phi(x + axis) - phi(x - axis);
        }   
        return vec::normalise(n);
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

    virtual vec_t<double, D> get_uniform_centre_of_mass(){
        return vec_t<double, D>();
    }

    // TODO: this isn't dimension independent! maybe it should be in a sub class?
    //       only problem is that it fucks with inheritance with sub classes that 
    //       are already dimension independent. Fuck OOP sometimes, honestly.  
    virtual mat3_t get_uniform_inertia_tensor(double mass){
        // TODO
        return mat3_t::identity(); 
    }
};

using sdf2_t = sdf_t<2>;
using sdf3_t = sdf_t<3>;
#endif
