#ifndef SERAPHIM_DEFORM_H
#define SERAPHIM_DEFORM_H

#include "maths/maths.h"

typedef enum deform_type_t {
	deform_type_control,
	deform_type_collision,
	deform_type_generic
} deform_type;

typedef struct deform_t {
	vec3 x0;
	vec3 x;
	vec3 v;
	vec3 p;
	double m;
	deform_type_t type;
} deform_t;

#define SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY 0.1

#endif
