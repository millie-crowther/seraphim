#ifndef SERAPHIM_RANDOM_H
#define SERAPHIM_RANDOM_H

#include <stdint.h>

#include "maths/maths.h"

typedef struct srph_random {
    uint64_t state;
    uint64_t sequence;
} srph_random;

void srph_random_default_seed(srph_random * r);
void srph_random_seed(srph_random * r, uint64_t state, uint64_t sequence);

uint32_t srph_random_u32(srph_random * r);

double srph_random_f64(srph_random * r);
double srph_random_f64_range(srph_random * r, double l, double u);
void srph_random_direction(srph_random *r, vec3 *x);

#endif
