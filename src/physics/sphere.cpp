#include "physics/sphere.h"

bool sphere_intersect(const sphere_t *a, const sphere_t *b) {
    return a->r + b->r >= vec3_distance(&a->c, &b->c);
}

bool sphere_contains(const sphere_t *a, const sphere_t *b) {
    return a->r >= vec3_distance(&a->c, &b->c) + b->r;
}

bool sphere_contains_point(const sphere_t *s, const vec3 *x) {
    return vec3_distance(&s->c, x) <= s->r;
}

void sphere_capture(sphere_t *a, const sphere_t *b) {
    if (sphere_contains(a, b)) {
        return;
    }

    vec3 ab, ax, bx, centre;
    vec3_subtract(&ab, &b->c, &a->c);
    vec3_normalize(&ab, &ab);

    vec3_multiply_f(&ax, &ab, -a->r);
    vec3_multiply_f(&bx, &ab, b->r);

    vec3_add(&ax, &ax, &a->c);
    vec3_add(&bx, &bx, &b->c);

    vec3_add(&centre, &ax, &bx);
    vec3_divide_f(&centre, &centre, 2);

    *a = {
        .c = centre,
        .r = vec3_distance(&ax, &bx) / 2.0,
    };
}
