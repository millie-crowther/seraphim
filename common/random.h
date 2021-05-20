#ifndef SERAPHIM_RANDOM_H
#define SERAPHIM_RANDOM_H

#include <stdint.h>

#include "maths.h"

typedef struct random_t {
    uint64_t state;
    uint64_t sequence;
} random_t;

void srph_random_default_seed(random_t *r);
void srph_random_seed(random_t *r, uint64_t state, uint64_t sequence);

uint32_t srph_random_u32(random_t *r);

double srph_random_f64(random_t *r);
double srph_random_f64_range(random_t *r, double l, double u);
void srph_random_direction(random_t *r, vec3 *x);

#endif
