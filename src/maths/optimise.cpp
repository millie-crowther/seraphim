#include "maths/optimise.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "core/constant.h"

#define N 3
#define MAX_ITERATIONS 100

#define ALPHA 1.0
#define GAMMA 2.0
#define RHO   0.5
#define SIGMA 0.5

static int comparator(const void *_a, const void *_b)
{
	double a = ((srph_opt_sample *) _a)->fx;
	double b = ((srph_opt_sample *) _b)->fx;
	return (a > b) - (a < b);
}

void srph_opt_nelder_mead(srph_opt_sample * s, srph_opt_func f, void *data,
			  const vec3 * _xs, double *t)
{
	double threshold = t == NULL ? -DBL_MAX : *t;

	srph_opt_sample xs[N + 1];

	for (int i = 0; i < N + 1; i++) {
		xs[i].x = _xs[i];
		xs[i].fx = f(data, &_xs[i]);
	}
	qsort(xs, N + 1, sizeof(*xs), comparator);

	for (int i = 0; i < MAX_ITERATIONS; i++) {
		// terminate
		bool should_terminate = true;
		for (int j = 0; j < N && should_terminate; j++) {
			double x = xs[0].x.v[j];

			for (int k = 1; k < N + 1; k++) {
				if (fabs(x - xs[k].x.v[j]) > epsilon) {
					should_terminate = false;
					break;
				}
			}
		}

		if (should_terminate || xs[0].fx < threshold) {
			break;
		}

		qsort(xs, N + 1, sizeof(*xs), comparator);

		// calculate centroid
		vec3 x0 = vec3_zero;
		for (int j = 0; j < N; j++) {
			vec3_add(&x0, &x0, &xs[j].x);
		}
		vec3_multiply_f(&x0, &x0, 1.0 / N);

		// reflection
		vec3 xr;
		vec3_subtract(&xr, &x0, &xs[N].x);
		vec3_multiply_f(&xr, &xr, ALPHA);
		vec3_add(&xr, &xr, &x0);
		double fxr = f(data, &xr);
		if (xs[0].fx <= fxr && fxr < xs[N - 1].fx) {
			xs[N].x = xr;
			xs[N].fx = fxr;
			continue;
		}
		// expansion
		if (fxr < xs[0].fx) {
			vec3 xe;
			vec3_subtract(&xe, &xr, &x0);
			vec3_multiply_f(&xe, &xe, GAMMA);
			vec3_add(&xe, &xe, &x0);

			double fxe = f(data, &xe);
			if (fxe < fxr) {
				xs[N].x = xe;
				xs[N].fx = fxe;
			} else {
				xs[N].x = xr;
				xs[N].fx = fxr;
			}
			continue;
		}
		// contraction
		vec3 xc;
		vec3_subtract(&xc, &xs[N].x, &x0);
		vec3_multiply_f(&xc, &xc, RHO);
		vec3_add(&xc, &xc, &x0);
		double fxc = f(data, &xc);
		if (fxc < xs[N].fx) {
			xs[N].x = xc;
			xs[N].fx = fxc;
			continue;
		}
		// shrink
		for (int j = 1; j < N + 1; j++) {
			vec3_subtract(&xs[j].x, &xs[j].x, &xs[0].x);
			vec3_multiply_f(&xs[j].x, &xs[j].x, SIGMA);
			vec3_add(&xs[j].x, &xs[j].x, &xs[0].x);
			xs[j].fx = f(data, &xs[j].x);
		}
	}

	*s = xs[0];
}
