#ifndef SERAPHIM_PRIMITIVE_H
#define SERAPHIM_PRIMITIVE_H

#include "sdf.h"
#include "platonic.h"

sdf_t *sdf_sphere_create(srph::seraphim_t * seraphim, double r);
sdf_t *sdf_torus_create(srph::seraphim_t * seraphim, double r1, double r2);

#endif
