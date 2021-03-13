#ifndef SERAPHIM_CONSTRAINT_H
#define SERAPHIM_CONSTRAINT_H

#include "core/array.h"

typedef struct srph_vertex {
    vec3 * x;
    vec3 d;
    vec3 v;
    double w;
} srph_vertex;

typedef double (*srph_constraint_func)(const srph_vertex ** xs, uint32_t n);
typedef void (*srph_constraint_derivative_func)(
    vec3 * dC, const srph_vertex ** xs, uint32_t i, uint32_t n
);

typedef struct srph_constraint {
    bool is_equality;
    double stiffness;
    srph_array vertex_pointers;

    srph_constraint_func c_func;
    srph_constraint_derivative_func dc_func;
} srph_constraint;

void srph_constraint_update(const srph_constraint * c, vec3 * dp, uint32_t i, double s); 
double srph_constraint_scaling_factor(const srph_constraint * c);

#endif
