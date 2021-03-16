#include "maths/vector.h"

#include <iostream>
#include <math.h>

#define VECTOR_UNI_OP(fx, x, f, n)     for (int i = 0; i < n; i++) fx[i] = f(x[i]);
#define VECTOR_BIN_OP(ab, a, b, op, n) for (int i = 0; i < n; i++) ab[i] = a[i] op b[i]; 

static double vector_dot(const double * as, const double * bs, int n){
    double l = 0.0;
    for (int i = 0; i < n; i++){
        l += as[i] * bs[i];
    }
    return l;
}

void srph_vec3_fill(vec3 * v, double x){
    *v = { x, x, x };
}

void srph_vec3_scale(vec3 * sx, const vec3 * x, double s){
    VECTOR_UNI_OP(sx->raw, x->raw, s *, 3); 
}

void srph_vec3_multiply(vec3 * x, const vec3 * a, const vec3 * b){
    VECTOR_BIN_OP(x->raw, a->raw, b->raw, *, 3);
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

void srph_vec3_normalise(vec3 * nx, const vec3 * x){
    double l = srph_vec3_length(x);

    if (l != 0.0){
        srph_vec3_scale(nx, x, 1.0 / l);
    }
}

void srph_vec3_subtract(vec3 * x, const vec3 * a, const vec3 * b){
    VECTOR_BIN_OP(x->raw, a->raw, b->raw, -, 3);
}

void srph_vec3_abs(vec3 * abs_x, const vec3 * x){
    VECTOR_UNI_OP(abs_x->raw, x->raw, fabs, 3);
}

void srph_vec3_print(const vec3 * x){
    printf("vec3(%f, %f, %f)", x->x, x->y, x->z);
}

void srph_vec3_negate(vec3 * nx, const vec3 * x){
    VECTOR_UNI_OP(nx->raw, x->raw, -, 3);
}

void srph_vec3_add(vec3 * x, const vec3 * a, const vec3 * b){
    VECTOR_BIN_OP(x->raw, a->raw, b->raw, +, 3);
}

void srph_vec3_divide(vec3 * x, const vec3 * a, const vec3 * b){
    VECTOR_BIN_OP(x->raw, a->raw, b->raw, /, 3);
}

void srph_vec3_max_scalar(vec3 * max_x, const vec3 * x, double m){
    for (int i = 0; i < 3; i++){
        max_x->raw[i] = fmax(x->raw[i], m);
    }
}

void srph_vec3_project(vec3 * pa, const vec3 * a, const vec3 * b){
    double s = srph_vec3_dot(a, b);
    double l = srph_vec3_length(b);
    vec3 a1;

    srph_vec3_scale(&a1, b, l == 0 ? s : s / l);
    srph_vec3_subtract(pa, a, &a1);
}

double srph_vec3_distance(const vec3 * a, const vec3 * b){
    vec3 d;
    srph_vec3_subtract(&d, a, b);
    return srph_vec3_length(&d);
}
   
void srph_vec3_cross(vec3 * axb, const vec3 * a, const vec3 * b){
    *axb = {
        a->y * b->z - a->z * b->y,
        a->z * b->x - a->x * b->z,
        a->x * b->y - a->y * b->x
    };
} 
