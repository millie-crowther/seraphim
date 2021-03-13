#include "physics/constraint.h"

double srph_constraint_scaling_factor(const srph_constraint * c){
    srph_vertex ** xs = (srph_vertex **) srph_array_first(&c->vertex_pointers);
    uint32_t n = c->vertex_pointers.size;    

    if (n == 0){
        return 0.0;
    }

    double s = c->c_func(xs, n);
    double q = 0.0;
    vec3 dc;
    for (uint32_t i = 0; i < n; i++){
        c->dc_func(&dc, xs, i, n);
        q += srph_vec3_dot(&c, &c) * xs[i]->w;
    }

    return s / q;
}

void srph_constraint_update(const srph_constraint * c, vec3 * dp, uint32_t i, double s){
    srph_vertex ** xs = (srph_vertex **) srph_array_first(&c->vertex_pointers);
    uint32_t n = c->vertex_points.size;
    dc_func(dp, xs, i, n);
    srph_vec3_scale(dp, dp, -s * xs[i]->w);
}
