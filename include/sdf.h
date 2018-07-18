#ifndef SDF_H
#define SDF_H

#include "vec.h"
#include <functional>


class sdf_t {
private:
    static constexpr float epsilon = 0.0001f;

    std::function<float(const vec3_t&)> phi;

public:
    sdf_t(std::function<float(const vec3_t&)> phi); 
    float operator()(const vec3_t& p);
    vec3_t normal(const vec3_t& p);
};

#endif
