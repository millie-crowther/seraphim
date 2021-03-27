#include "physics/constraint.h"

#include <assert.h>
#include <stdlib.h>

double srph_constraint_scaling_factor(srph_constraint * c){
    assert(c != NULL);

    if (c->n == 0 || c->stiffness <= 0){
        return 0.0;
    }

    double s = c->c_func(c);

    if (s == 0.0 || (!c->is_equality && s < 0)){
        return 0.0;
    }
        

    double q = 0.0;
    vec3 dc;
    for (uint32_t i = 0; i < c->n; i++){
        c->dc_func(c, i, &dc);
        q += srph_vec3_dot(&dc, &dc) / c->deformations[i]->m;
    }

    if (q == 0.0){
        return 0.0;
    }

    return s / q;
}

void srph_constraint_update(srph_constraint * c, uint32_t i, double s){
    assert(c != NULL);

    if (i >= c->n || s == 0.0){
        return;
    }

    vec3 dp;
    vec3 * p = &c->deformations[i]->p;
    c->dc_func(c, i, &dp);
    srph_vec3_scale(&dp, &dp, -s / c->deformations[i]->m);
    srph_vec3_add(p, p, &dp);
}

static double distance_constraint(srph_constraint * c){
    assert(c != NULL && c->deformations[0] != NULL && c->deformations[1] != NULL);
    
    double l  = srph_vec3_distance(&c->deformations[0]->p,  &c->deformations[1]->p );
    double l0 = srph_vec3_distance(&c->deformations[0]->x0, &c->deformations[1]->x0);
    
    return l - l0;
}

static void distance_constraint_derivative(srph_constraint * c, uint32_t i, vec3 * dc){
    assert(c != NULL && dc != NULL && c->deformations[0] != NULL && c->deformations[1] != NULL);
    srph_vec3_subtract(dc, &c->deformations[i]->p, &c->deformations[1 - i]->p);
    srph_vec3_normalise(dc, dc);
}

void srph_constraint_distance(
    srph_constraint * c, srph_deform * a, srph_deform * b, double stiffness
){
    assert(c != NULL && a != NULL && b != NULL);
    
    *c = {
        .is_equality = true,
        .stiffness = stiffness,
        .data = NULL,
        .n = 2,
        .c_func = distance_constraint,
        .dc_func = distance_constraint_derivative,
    };
    c->deformations[0] = a;
    c->deformations[1] = b;
}
