#include "maths/sdf/sdf.h"

#include <core/seraphim.h>
#include <stdlib.h>

#include "core/random.h"
#include "physics/sphere.h"

#include "core/constant.h"

void sdf_create(uint32_t *id, sdf_t *sdf, sdf_func_t phi, void *data) {
    sdf->distance_function = phi;
    sdf->data = data;
    sdf->id = *id;
    (*id)++;

    sdf->is_bound_valid = false;
    sdf->is_com_valid = false;
    sdf->is_inertia_tensor_valid = false;
    sdf->volume = -1.0;
    sdf->is_convex = false;

    // TODO: improve CoM calculation and remove this
    sdf->com = vec3_zero;
    sdf->is_com_valid = true;
}

double sdf_distance(sdf_t *sdf, const vec3 *x) {
    return sdf->distance_function(sdf->data, x);
}

vec3 sdf_normal(sdf_t *sdf, const vec3 *x) {
    vec3 n;
    for (int i = 0; i < 3; i++) {
        vec3 x1 = *x;
        x1.v[i] += epsilon;

        vec3 x2 = *x;
        x2.v[i] -= epsilon;

        n.v[i] = sdf_distance(sdf, &x1) - sdf_distance(sdf, &x2);
    }

    vec3_multiply_f(&n, &n, 0.5 / epsilon);

    return n;
}

bool sdf_contains(sdf_t *sdf, const vec3 *x) {
    return srph_bound3_contains(&sdf->bound, x) && sdf_distance(sdf, x) <= 0.0;
}

double srph_sdf_project(sdf_t *sdf, const vec3 *d) {
    vec3 x;
    vec3_normalize(&x, d);
    vec3_multiply_f(&x, &x, rho);

    while (true) {
        double p = sdf_distance(sdf, &x);

        // TODO: better quit criteria
        if (p > rho) {
            return vec3_length(&x) - p;
        }

        vec3_multiply_f(&x, &x, 2.0);
    }
}

double sdf_volume(sdf_t *sdf) {
    if (sdf->volume < 0.0) {
        int hits = 0;

        bound3_t *b = sdf_bound(sdf);
        random_t rng;
        srph_random_default_seed(&rng);

        while (hits < SERAPHIM_SDF_VOLUME_SAMPLES) {
            vec3 x;
            x.x = srph_random_f64_range(&rng, b->lower.x, b->upper.x);
            x.y = srph_random_f64_range(&rng, b->lower.y, b->upper.y);
            x.z = srph_random_f64_range(&rng, b->lower.z, b->upper.z);

            if (sdf_contains(sdf, &x)) {
                hits++;
            }
        }

        sdf->volume = srph_bound3_volume(b) * (double)hits /
                      (double)SERAPHIM_SDF_VOLUME_SAMPLES;
    }

    return sdf->volume;
}

bound3_t *sdf_bound(sdf_t *sdf) {
    if (sdf == NULL) {
        return NULL;
    }

    if (!sdf->is_bound_valid) {
        vec3 a;
        for (int i = 0; i < 3; i++) {
            a = vec3_zero;

            a.v[i] = -1.0;
            sdf->bound.lower.v[i] = -srph_sdf_project(sdf, &a);

            a.v[i] = 1.0;
            sdf->bound.upper.v[i] = srph_sdf_project(sdf, &a);
        }

        sdf->is_bound_valid = true;
    }

    return &sdf->bound;
}

double sdf_discontinuity(sdf_t *sdf, const vec3 *x) {
    vec3 ns[3];

    for (int axis = 0; axis < 3; axis++) {
        vec3 x1 = *x;
        x1.v[axis] -= epsilon;
        vec3 x2 = *x;
        x2.v[axis] += epsilon;

        vec3 n1 = sdf_normal(sdf, &x1);
        vec3 n2 = sdf_normal(sdf, &x2);
        vec3_subtract(&ns[axis], &n2, &n1);

        vec3_divide_f(&ns[axis], &ns[axis], epsilon * 2);
    }

    return fabs(mat3_determinant((mat3 *)ns));
}