#ifndef SERAPHIM_PRIMITIVE_H
#define SERAPHIM_PRIMITIVE_H

#include "sdf.h"

void sdf_sphere_create(uint32_t * id, sdf_t * sdf, double *r);
void sdf_torus_create(uint32_t * id, sdf_t * sdf, vec2 * rs);

#endif
