#include "maths/optimise.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "core/constant.h"

#define N 3
#define MAX_ITERATIONS 100

#define ALPHA 1.0
#define GAMMA 2.0
#define RHO 0.5
#define SIGMA 0.5

static int comparator(const void *_a, const void *_b) {
    double a = ((opt_sample_t *)_a)->fx;
    double b = ((opt_sample_t *)_b)->fx;
    return (a > b) - (a < b);
}

void opt_nelder_mead(opt_sample_t *s, opt_func_t f, void *data,
                     const vec3 *xs, const double *threshold_) {
    double threshold = threshold_ == NULL ? -DBL_MAX : *threshold_;

    opt_sample_t samples[N + 1];

    for (int i = 0; i < N + 1; i++) {
        samples[i].x = xs[i];
        samples[i].fx = f(data, &xs[i]);
    }
    qsort(samples, N + 1, sizeof(*samples), comparator);

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        // terminate
        bool should_terminate = true;
        for (int j = 0; j < N && should_terminate; j++) {
            double x = samples[0].x.v[j];

            for (int k = 1; k < N + 1; k++) {
                if (fabs(x - samples[k].x.v[j]) > epsilon) {
                    should_terminate = false;
                    break;
                }
            }
        }

        if (should_terminate || samples[0].fx < threshold) {
            break;
        }

        qsort(samples, N + 1, sizeof(*samples), comparator);

        // calculate centroid
        vec3 x0 = vec3_zero;
        for (int j = 0; j < N; j++) {
            vec3_add(&x0, &x0, &samples[j].x);
        }
        vec3_multiply_f(&x0, &x0, 1.0 / N);

        // reflection
        vec3 xr;
        vec3_subtract(&xr, &x0, &samples[N].x);
        vec3_multiply_f(&xr, &xr, ALPHA);
        vec3_add(&xr, &xr, &x0);
        double fxr = f(data, &xr);
        if (samples[0].fx <= fxr && fxr < samples[N - 1].fx) {
            samples[N].x = xr;
            samples[N].fx = fxr;
            continue;
        }
        // expansion
        if (fxr < samples[0].fx) {
            vec3 xe;
            vec3_subtract(&xe, &xr, &x0);
            vec3_multiply_f(&xe, &xe, GAMMA);
            vec3_add(&xe, &xe, &x0);

            double fxe = f(data, &xe);
            if (fxe < fxr) {
                samples[N].x = xe;
                samples[N].fx = fxe;
            } else {
                samples[N].x = xr;
                samples[N].fx = fxr;
            }
            continue;
        }
        // contraction
        vec3 xc;
        vec3_subtract(&xc, &samples[N].x, &x0);
        vec3_multiply_f(&xc, &xc, RHO);
        vec3_add(&xc, &xc, &x0);
        double fxc = f(data, &xc);
        if (fxc < samples[N].fx) {
            samples[N].x = xc;
            samples[N].fx = fxc;
            continue;
        }
        // shrink
        for (int j = 1; j < N + 1; j++) {
            vec3_subtract(&samples[j].x, &samples[j].x, &samples[0].x);
            vec3_multiply_f(&samples[j].x, &samples[j].x, SIGMA);
            vec3_add(&samples[j].x, &samples[j].x, &samples[0].x);
            samples[j].fx = f(data, &samples[j].x);
        }
    }

    *s = samples[0];
}
