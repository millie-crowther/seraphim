#include "maths/sdf/primitive.h"

#include <math.h>

double sphere_phi(void * data, const vec3 * x){
    double r = *((double *) data);
    return srph_vec3_length(x) - r;
}

double torus_phi(void * data, const vec3 * x){
    srph_sdf_torus * t = (srph_sdf_torus *) data;
    vec2 xy;
    xy.x = hypot(x->x, x->z) - t->_r1;
    xy.y = x->y;
    return srph_vec2_length(&xy) - t->_r2;
}

void srph_sdf_sphere_create(srph_sdf_sphere * sphere, double r){
    if (sphere == NULL){
        return;
    }

    sphere->_r = r;

    srph::mat3_t inertia_tensor = srph::mat3_t::diagonal(0.4 * std::pow(r, 2));
    srph_sdf_full_create(&sphere->sdf, sphere_phi, (void *) &sphere->_r, &inertia_tensor); 
}

void srph_sdf_torus_create(srph_sdf_torus * t, double r1, double r2){
    if (t == NULL){
        return;
    }

    t->_r1 = r1;
    t->_r2 = r2;
    
    double xz = 1.0 / 8.0 * (4.0 * pow(r1, 2) + 3.0 * pow(r2, 2)); 
    double y = 1.0 / 4.0 * (4.0 * pow(r1, 2) + 5.0 * pow(r2, 2));
    srph::mat3_t inertia_tensor(
        xz, 0, 0,
        0,  y, 0,
        0,  0, xz
    );
    srph_sdf_full_create(&t->sdf, torus_phi, (void *) t, &inertia_tensor); 
}
