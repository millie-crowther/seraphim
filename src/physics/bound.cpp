#include "physics/bound.h"

#include <float.h>
#include <math.h>

#include <iostream>

void bound3_create(bound3_t *b) {
    for (int i = 0; i < 3; i++) {
        b->lower.v[i] = DBL_MAX;
        b->upper.v[i] = -DBL_MAX;
    }
}

void bound3_intersection(const bound3_t *a, const bound3_t *b,
                         bound3_t *intersection) {
    for (int i = 0; i < 3; i++) {
        intersection->lower.v[i] = fmax(a->lower.v[i], b->lower.v[i]);
        intersection->upper.v[i] = fmin(a->upper.v[i], b->upper.v[i]);
    }
}

void bound3_vertex(const bound3_t *b, int i, vec3 *v) {
    for (int i = 0; i < 3; i++) {
        if ((i & (1 << i)) != 0) {
            v->v[i] = b->upper.v[i];
        } else {
            v->v[i] = b->lower.v[i];
        }
    }
}

void bound3_midpoint(const bound3_t *b, vec3 *v) {
    for (int i = 0; i < 3; i++) {
        v->v[i] = (b->upper.v[i] + b->lower.v[i]) / 2.0;
    }
}

void bound3_radius(const bound3_t *b, vec3 *v) {
    for (int i = 0; i < 3; i++) {
        v->v[i] = (b->upper.v[i] - b->lower.v[i]) / 2.0;
    }
}

void bound3_capture(bound3_t *b, vec3 *v) {
    for (int i = 0; i < 3; i++) {
        b->lower.v[i] = fmin(b->lower.v[i], v->v[i]);
        b->upper.v[i] = fmax(b->upper.v[i], v->v[i]);
    }
}

double bound3_volume(const bound3_t *b) {
    double v = 1.0;
    for (int i = 0; i < 3; i++) {
        v *= b->upper.v[i] - b->lower.v[i];
    }
    return v;
}

bool bound3_contains(bound3_t *b, const vec3 *v) {
    for (int i = 0; i < 3; i++) {
        if (v->v[i] < b->lower.v[i] || v->v[i] > b->upper.v[i]) {
            return false;
        }
    }
    return true;
}

void bound3_bisect(const bound3_t *self, bound3_t *sub_bounds) {
    vec3 radius;
    int max_axis = 0;

    bound3_radius(self, &radius);
    for (int axis = 1; axis < 3; axis++) {
        if (radius.v[axis] > radius.v[max_axis]) {
            max_axis = axis;
        }
    }

    sub_bounds[0] = *self;
    sub_bounds[1] = *self;

    double midpoint = (self->upper.v[max_axis] + self->lower.v[max_axis]) / 2.0;
    sub_bounds[0].upper.v[max_axis] = midpoint;
    sub_bounds[1].lower.v[max_axis] = midpoint;
}

bool bound3_is_valid(bound3_t *b) {
    if (b == NULL) {
        return false;
    }

    for (int i = 0; i < 3; i++) {
        if (b->lower.v[i] > b->upper.v[i]) {
            return false;
        }
    }

    return true;
}
