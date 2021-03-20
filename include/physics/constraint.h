#ifndef SERAPHIM_CONSTRAINT_H
#define SERAPHIM_CONSTRAINT_H

#include <stdint.h>

#include "core/array.h"
#include "maths/vector.h"

#define SERAPHIM_MAX_VERTICES_PER_CONSTRAINT 4

typedef struct srph_vertex {
    vec3 x0;
    vec3 x;
    vec3 v;
    vec3 p;
    double m;
} srph_vertex;

struct srph_constraint;
typedef double (*srph_constraint_func)(struct srph_constraint * c);
typedef void (*srph_constraint_derivative_func)(struct srph_constraint * c, uint32_t i, vec3 * dC);

typedef struct srph_constraint {
    bool _is_equality;
    double _stiffness;
    void * _data;
    srph_constraint_func _c_func;
    srph_constraint_derivative_func _dc_func;
    
    uint32_t n;
    srph_vertex * _vertices[SERAPHIM_MAX_VERTICES_PER_CONSTRAINT];
} srph_constraint;

typedef srph_array(srph_constraint) srph_constraint_array;

void srph_constraint_init(
    srph_constraint * c, bool is_equality, double stiffness, uint32_t n,
    void * data, srph_constraint_func c_func, srph_constraint_derivative_func dc_func
);

void srph_constraint_update(srph_constraint * c, uint32_t i, double s); 
double srph_constraint_scaling_factor(srph_constraint * c);

#endif
