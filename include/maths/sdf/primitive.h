#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "sdf.h"

typedef struct srph_sdf_sphere {
    srph_sdf sdf;
    double _r;
} srph_sdf_sphere;

typedef struct srph_sdf_torus {
    srph_sdf sdf;
    double _r1;
    double _r2;
} srph_sdf_torus;

void srph_sdf_sphere_create(srph_sdf_sphere * sdf, double r);
void srph_sdf_torus_create(srph_sdf_torus * sdf, double r1, double r2);

#endif
