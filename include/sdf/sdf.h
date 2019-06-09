#ifndef SDF_H
#define SDF_H

#include "maths/vec.h"

class sdf_t {
protected:
    // protected constructor for abstract class
    sdf_t(){}

public:
    // accessors 
    virtual double phi(const vec3_t & x) const = 0;
    virtual vec3_t normal(const vec3_t & x) const;

    // overloaded operators
    // sdf_t operator&(const sdf_t & sdf) const;
    // sdf_t operator|(const sdf_t & sdf) const;
    // sdf_t operator!() const;
    // sdf_t operator-(const sdf_t & sdf) const;
};

#endif
