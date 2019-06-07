#ifndef SDF_H
#define SDF_H

#include <functional>

#include "maths/vec.h"

/*
    - continuity
    - level set property regarding gradient magnitude
*/
class sdf_t {
private:
    typedef std::function<double(const vec3_t &)> phi_t;

    phi_t phi;

public:
    // constructors and destructors
    sdf_t(const phi_t & phi); 

    // accessors
    vec3_t normal(const vec3_t & p) const;

    // overloaded operators
    double operator()(const vec3_t & v) const;
    sdf_t operator&(const sdf_t & sdf) const;
    sdf_t operator|(const sdf_t & sdf) const;
    sdf_t operator!() const;
    sdf_t operator-(const sdf_t & sdf) const;
};

#endif
