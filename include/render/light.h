#ifndef LIGHT_H
#define LIGHT_H

#include "maths/matrix.h"

typedef struct light_t {
    srph::f32vec3_t x;
    uint32_t id;

    srph::f32vec4_t colour;

    light_t();
    light_t(const srph::f32vec3_t &x, const srph::f32vec4_t &colour);
} light_t;

#endif
