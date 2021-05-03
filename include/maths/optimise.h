#ifndef SERAPHIM_OPTIMISE_H
#define SERAPHIM_OPTIMISE_H

#include "maths/maths.h"

typedef double (*srph_opt_func)(void *data, const vec3 *x);

typedef struct srph_opt_sample {
    vec3 x;
    double fx;
} srph_opt_sample;

void srph_opt_nelder_mead(srph_opt_sample *s, srph_opt_func f, void *data,
                          const vec3 *xs, double *threshold);

#endif
