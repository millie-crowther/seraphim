#ifndef SDF_H
#define SDF_H

#include "maths/vec.h"

template<uint8_t D>
class sdf_t {
protected:
    // protected constructor for abstract class
    sdf_t(){}

public:
    // virtual accessors 
    virtual double phi(const vec_t<double, D> & x) const = 0;

    virtual vec_t<double, D> normal(const vec_t<double, D> & x) const {
        vec_t<double, D> result;
        vec_t<double, D> axis;

        for (uint8_t i = 0; i < D; i++){
            axis = vec_t<double, D>::axis(i) * constant::epsilon;
            result[i] = phi(x + axis) - phi(x - axis);
        }

        // TODO: if SDFs obey level set property, this can just be a scale rather than expensive sqrt
        return result.normalise();
    }

    // concrete accessors
    vec_t<double, D+1> plane(const vec_t<double, D> & x) const {
        vec_t<double, D+1> result;

        vec_t<double, D> n = normal(x);
        for (uint8_t i = 0; i < D; i++){
            result[i] = n[i];
        }
        result[D] = (x * n) - phi(x);
        return result;
    }
};

typedef sdf_t<2> sdf2_t;
typedef sdf_t<3> sdf3_t;

#endif
