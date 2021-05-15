#ifndef LIGHT_H
#define LIGHT_H

#include "maths/maths.h"

typedef struct light_t {
    vec3f x;
    uint32_t id;

    vec4f colour;

    light_t();
    light_t(vec3f *x, vec4f *colour);
} light_t;

#endif
