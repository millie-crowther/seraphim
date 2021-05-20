#ifndef SERAPHIM_OPTIMISE_H
#define SERAPHIM_OPTIMISE_H

#include "../common/maths.h"

typedef double (*opt_func_t)(void *data, const vec3 *x);

typedef struct opt_sample_t {
    vec3 x;
    double fx;
} opt_sample_t;

void opt_nelder_mead(opt_sample_t *s, opt_func_t f, void *data,
                     const vec3 *xs, const double *threshold_);

#endif
