#ifndef SDF_H
#define SDF_H

#include "vec.h"
#include <functional>

class bounds_t;

class sdf_t {
private:
    std::function<float(const vec3_t&)> phi;

public:
    // constructors and destructors
    sdf_t(std::function<float(const vec3_t&)> phi); 

    // accessors
    vec3_t normal(const vec3_t& p) const;
    double distance(const vec3_t& v) const;
    double operator()(const vec3_t& v) const;
 
    bounds_t get_bounds() const;
};

#endif
