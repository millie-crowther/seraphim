#include "physics/constraint.h"

#include <stdlib.h>

void srph_constraint_init(
    srph_constraint * c, bool is_equality, double stiffness, uint32_t n,
    void * data, srph_constraint_func c_func, srph_constraint_derivative_func dc_func
){
    c->_is_equality = is_equality;
    c->_stiffness = stiffness;
    c->_data = data;
    c->_c_func = c_func;
    c->_dc_func = dc_func;
    c->n = n; 
}

double srph_constraint_scaling_factor(srph_constraint * c){
    if (c->n == 0){
        return 0.0;
    }

    double s = c->_c_func(c);
    double q = 0.0;
    vec3 dc;
    for (uint32_t i = 0; i < c->n; i++){
        c->_dc_func(c, i, &dc);
        q += srph_vec3_dot(&dc, &dc) / c->_vertices[i]->m;
    }

    if (q == 0.0){
        return 0.0;
    }

    return s / q;
}

void srph_constraint_update(srph_constraint * c, vec3 * dp, uint32_t i, double s){
    if (c->n == 0){
        srph_vec3_fill(dp, 0.0);
        return;
    }

    c->_dc_func(c, i, dp);
    srph_vec3_scale(dp, dp, -s / c->_vertices[i]->m);
}
