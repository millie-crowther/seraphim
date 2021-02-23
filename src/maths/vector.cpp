#include "maths/vector.h"

#include <iostream>
#include <math.h>

static void vector_scale(double * xs, double s, int n){
    for (int i = 0; i < n; i++){
        xs[i] *= s;
    }
}

static double vector_dot(const double * as, const double * bs, int n){
    double l = 0.0;
    for (int i = 0; i < n; i++){
        l += as[i] * bs[i];
    }
    return l;
}

static void vector_multiply(double * xs, const double * as, const double * bs, int n){
    for (int i = 0; i < n; i++){
        xs[i] = as[i] * bs[i];
    }
}

static void vector_abs(double * xs, int n){
    for (int i = 0; i < n; i++){
        xs[i] = fabs(xs[i]);
    }
}

void srph_vec3_fill(vec3 * v, double x){
    srph_vec3_set(v, x, x, x);
}

void srph_vec3_set(vec3 * v, double x, double y, double z){
    v->x = x;
    v->y = y;
    v->z = z;
}

void srph_vec3_scale(vec3 * v, double s){
    vector_scale(v->raw, s, 3);
}

void srph_vec3_multiply(vec3 * x, const vec3 * a, const vec3 * b){
    vector_multiply(x->raw, a->raw, b->raw, 3);
}

double srph_vec3_dot(const vec3 * a, const vec3 * b){
    return vector_dot(a->raw, b->raw, 3);
}

double srph_vec3_length(const vec3 * x){
    return sqrt(vector_dot(x->raw, x->raw, 3));
}

double srph_vec2_length(const vec2 * x){ 
    return sqrt(vector_dot(x->raw, x->raw, 2));
}

void srph_vec3_normalise(vec3 * x){
    double l = srph_vec3_length(x);

    if (l != 0.0){
        srph_vec3_scale(x, 1.0 / l);
    }
}

void srph_vec3_subtract(vec3 * x, const vec3 * a, const vec3 * b){
    for (int i = 0; i < 3; i++){
        x->raw[i] = a->raw[i] - b->raw[i];
    }
}

void srph_vec3_abs(vec3 * x){
    vector_abs(x->raw, 3);
}

void srph_vec3_print(const vec3 * x){
    printf("vec3(%f, %f, %f)", x->x, x->y, x->z);
}

void srph_vec3_add(vec3 * x, const vec3 * a, const vec3 * b){
    for (int i = 0; i < 3; i++){
        x->raw[i] = a->raw[i] + b->raw[i];
    }
}
