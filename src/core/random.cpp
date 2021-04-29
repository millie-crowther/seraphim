#include "core/random.h"

#include <math.h>
#include <time.h>

#include <iostream>

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
uint32_t srph_random_u32(random_t * r) {
	uint64_t oldstate = r->state;

	// Advance internal state
	r->state = oldstate * 6364136223846793005ULL + r->sequence;

	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void srph_random_seed(random_t * r, uint64_t state, uint64_t sequence) {
	if (r == NULL) {
		return;
	}

	r->state = state;
	r->sequence = sequence | 1;
	srph_random_u32(r);
}

void srph_random_default_seed(random_t * r) {
	srph_random_seed(r, time(NULL), (uint64_t) r);
}

double srph_random_f64(random_t * r) {
	union {
		uint32_t u[2];
		double d;
	} u {
	};

	do {
		u.u[0] = srph_random_u32(r);
		u.u[1] = srph_random_u32(r);
	}
	while (!isfinite(u.d));

	static int _;
	double d = frexp(u.d, &_);
	d = fabs(d) - 0.5;
	d *= 2.0;

	return d;
}

double srph_random_f64_range(random_t * r, double l, double u) {
	return srph_random_f64(r) * (u - l) + l;
}

void srph_random_direction(random_t * r, vec3 * x) {
	do {
		x->x = srph_random_f64_range(r, -1, 1);
		x->y = srph_random_f64_range(r, -1, 1);
		x->z = srph_random_f64_range(r, -1, 1);
	}
	while (vec3_length(x) >= 1.0);
	vec3_normalize(x, x);
}
