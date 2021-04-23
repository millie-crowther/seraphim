#include "maths/bound.h"

#include <float.h>
#include <math.h>

#include <iostream>

void srph_bound3_create(srph_bound3 * b){
    if (b == NULL){
        return;
    }

    for (int i = 0; i < 3; i++){
        b->lower.v[i] =  DBL_MAX;
        b->upper.v[i] = -DBL_MAX;
    } 
}

void srph_bound3_intersection(const srph_bound3 * a, const srph_bound3 * b, srph_bound3 * intersect){
    for (int i = 0; i < 3; i++){
        intersect->lower.v[i] = fmax(a->lower.v[i], b->lower.v[i]);
        intersect->upper.v[i] = fmin(a->upper.v[i], b->upper.v[i]);
    }     
}

void srph_bound3_vertex(const srph_bound3 * b, int vertex_index, vec3 * v){
    for (int i = 0; i < 3; i++){
        if ((vertex_index & (1 << i)) != 0){
            v->v[i] = b->upper.v[i];
        } else {
            v->v[i] = b->lower.v[i];
        }
    } 
}

void srph_bound3_midpoint(const srph_bound3 * b, vec3 * v){
    for (int i = 0; i < 3; i++){
        v->v[i] = (b->upper.v[i] + b->lower.v[i]) / 2.0;
    }
}

void srph_bound3_radius(const srph_bound3 * b, vec3 * v){
    for (int i = 0; i < 3; i++){
        v->v[i] = (b->upper.v[i] - b->lower.v[i]) / 2.0;
    }
}

void srph_bound3_capture(srph_bound3 * b, vec3 * v){
    for (int i = 0; i < 3; i++){
        b->lower.v[i] = fmin(b->lower.v[i], v->v[i]);
        b->upper.v[i] = fmax(b->upper.v[i], v->v[i]);
    }
}

double srph_bound3_volume(const srph_bound3 * b){
    double v = 1.0;
    for (int i = 0; i < 3; i++){
        v *= b->upper.v[i] - b->lower.v[i];
    }
    return v;
}

bool srph_bound3_contains(srph_bound3 *b, const vec3 *v) {
    for (int i = 0; i < 3; i++){
        if (v->v[i] < b->lower.v[i] || v->v[i] > b->upper.v[i]){
            return false;
        }
    }
    return true;
}

void srph_bound3_bisect(const srph_bound3 *self, srph_bound3 *sub_bounds) {
    vec3 radius;
    int max_axis = 0;

    srph_bound3_radius(self, &radius);
    for (int axis = 1; axis < 3; axis++){
        if (radius.v[axis] > radius.v[max_axis]){
            max_axis = axis;
        }
    }

    sub_bounds[0] = *self;
    sub_bounds[1] = *self;

    double midpoint = (self->upper.v[max_axis] + self->lower.v[max_axis]) / 2.0;
    sub_bounds[0].upper.v[max_axis] = midpoint;
    sub_bounds[1].lower.v[max_axis] = midpoint;
}
