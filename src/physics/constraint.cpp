#include "physics/constraint.h"

#include <stdlib.h>

void srph_constraint_init(
    srph_constraint * c, bool is_equality, double stiffness,
    void * data, srph_constraint_func c_func, srph_constraint_derivative_func dc_func
){
    c->_is_equality = is_equality;
    c->_stiffness = stiffness;
    c->_data = data;
    c->_c_func = c_func;
    c->_dc_func = dc_func; 

    srph_array_create(&c->_vertex_pointers, sizeof(srph_vertex *));
}

void srph_constraint_destroy(srph_constraint * c){
    if (c != NULL){
        srph_array_destroy(&c->_vertex_pointers);
    }
}

double srph_constraint_scaling_factor(const srph_constraint * c){
    if (srph_array_is_empty(&c->_vertex_pointers)){
        return 0.0;
    }

    srph_vertex ** xs = (srph_vertex **) srph_array_first(&c->_vertex_pointers);
    uint32_t n = c->_vertex_pointers.size;    
    double s = c->_c_func(c->_data, xs, n);
    double q = 0.0;
    vec3 dc;
    for (uint32_t i = 0; i < n; i++){
        c->_dc_func(c->_data, &dc, xs, i, n);
        q += srph_vec3_dot(&dc, &dc) * xs[i]->w;
    }

    if (q == 0.0){
        return 0.0;
    }

    return s / q;
}

void srph_constraint_update(const srph_constraint * c, vec3 * dp, uint32_t i, double s){
    srph_vertex ** xs = (srph_vertex **) srph_array_first(&c->_vertex_pointers);
    uint32_t n = c->_vertex_pointers.size;
    c->_dc_func(c->_data, dp, xs, i, n);
    srph_vec3_scale(dp, dp, -s * xs[i]->w);
}
