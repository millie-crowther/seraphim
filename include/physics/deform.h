#ifndef SERAPHIM_DEFORM_H
#define SERAPHIM_DEFORM_H

#include "maths/maths.h"

typedef enum srph_deform_type {
    srph_deform_type_control,
    srph_deform_type_collision,
    srph_deform_type_generic
} srph_deform_type;

typedef struct srph_deform {
    vec3 x0;
    vec3 x;
    vec3 v;
    vec3 p;
    double m;
    srph_deform_type type;
} srph_deform;

#define SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY 0.1

#endif
