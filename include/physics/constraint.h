#ifndef SERAPHIM_CONSTRAINT_H
#define SERAPHIM_CONSTRAINT_H

#include "core/array.h"
#include "maths/vector.h"

typedef struct srph_vertex {
    vec3 * _x_key;
    vec3 x;
    vec3 v;
    double w;
} srph_vertex;

typedef double (*srph_constraint_func)(const void * data, srph_vertex ** xs, uint32_t n);
typedef void (*srph_constraint_derivative_func)(
    const void * data, vec3 * dC, srph_vertex ** xs, uint32_t i, uint32_t n
);

typedef struct srph_constraint {
    bool _is_equality;
    double _stiffness;
    srph_array _vertex_pointers;
    void * _data;
    srph_constraint_func _c_func;
    srph_constraint_derivative_func _dc_func;
} srph_constraint;

void srph_constraint_init(
    srph_constraint * c, bool is_equality, double stiffness,
    void * data, srph_constraint_func c_func, srph_constraint_derivative_func dc_func
);
void srph_constraint_destroy(srph_constraint * c);

void srph_constraint_update(const srph_constraint * c, vec3 * dp, uint32_t i, double s); 
double srph_constraint_scaling_factor(const srph_constraint * c);

#endif
