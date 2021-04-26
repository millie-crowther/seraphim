#ifndef SERAPHIM_PLATONIC_H
#define SERAPHIM_PLATONIC_H

#include <core/seraphim.h>
#include "sdf.h"

sdf_t *sdf_cuboid_initialise(srph::seraphim_t *seraphim, const vec3 *r);
sdf_t *sdf_octahedron_initialise(srph::seraphim_t *seraphim, double e);

#endif
