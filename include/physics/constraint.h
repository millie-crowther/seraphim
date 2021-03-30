#ifndef SERAPHIM_CONSTRAINT_H
#define SERAPHIM_CONSTRAINT_H

#include <stdint.h>

#include "core/array.h"
#include "maths/vector.h"
#include "physics/deform.h"

#define SERAPHIM_MAX_DEFORMATIONS_PER_CONSTRAINT 4

struct srph_constraint;
typedef double (*srph_constraint_func)(struct srph_constraint * c);
typedef void (*srph_constraint_derivative_func)(struct srph_constraint * c, uint32_t i, vec3 * dC);

typedef struct srph_constraint {
    bool is_equality;
    double stiffness;
    uint32_t n;
    srph_constraint_func c_func;
    srph_constraint_derivative_func dc_func;
    srph_deform * deformations[SERAPHIM_MAX_DEFORMATIONS_PER_CONSTRAINT];
} srph_constraint;

typedef srph_array(srph_constraint) srph_constraint_array;

void srph_constraint_update(srph_constraint * c, uint32_t i, double s); 
double srph_constraint_scaling_factor(srph_constraint * c);

void srph_constraint_distance(
    srph_constraint * c, srph_deform * a, srph_deform * b, double stiffness
);

void srph_constraint_repulse_derivative(srph_constraint * c, uint32_t i, vec3 * dc);

#endif
