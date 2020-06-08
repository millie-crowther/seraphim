#ifndef LIGHT_H
#define LIGHT_H

#include <maths/vec.h>

class light_t {
private:
    f32vec3_t x;
    float _1;

    f32vec4_t colour;

public:
    light_t();
    light_t(const f32vec3_t & x, const f32vec4_t & colour);
};

#endif