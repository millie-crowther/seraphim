#ifndef SDF_H
#define SDF_H

#include "vec.h"
#include <functional>

class bounds_t;

class sdf_t {
private:
    static constexpr float epsilon = 0.0001f;

    std::function<float(const vec3_t&)> phi;

public:
    // constructors and destructors
    sdf_t(std::function<float(const vec3_t&)> phi); 

    // accessors
    vec3_t normal(const vec3_t& p) const;
    float distance(const vec3_t& v) const;
 
    // virtuals
    virtual bounds_t get_bounds() const;
};

#endif
