#ifndef SDF_H
#define SDF_H

#include "vec.h"
#include <functional>
#include "bounds.h"

class sdf_t {
private:
    static constexpr float epsilon = 0.0001f;

    std::function<float(const vec3_t&)> phi;

public:
    // constructors and destructors
    sdf_t(std::function<float(const vec3_t&)> phi); 

    // publids methods
    vec3_t normal(const vec3_t& p);
 
    // virtual methods
    virtual bounds_t get_bounds();
    
    // overloaded operators
    float operator()(const vec3_t& p);
};

#endif
