#include "maths/sdf/primitive.h"

#include <math.h>
#include <stdlib.h>

double sphere_phi(void * data, const vec3 * x){
    double r = *((double *) data);
    return srph_vec3_length(x) - r;
}

double torus_phi(void * data, const vec3 * x){
    double * rs = (double *) data;
    vec2 xy;
    xy.x = hypot(x->x, x->z) - rs[0];
    xy.y = x->y;
    return srph_vec2_length(&xy) - rs[1];
}

srph_sdf * srph_sdf_sphere_create(double r){
    srph_sdf * sdf = (srph_sdf *) malloc(sizeof(srph_sdf));
    if (sdf == NULL){
        return NULL;
    }

    double * r2 = (double *) malloc(sizeof(double));
    if (r2 == NULL){
        free(sdf);
        return NULL;
    }
    *r2 = r;

    srph::mat3_t inertia_tensor = srph::mat3_t::diagonal(0.4 * std::pow(r, 2));
    srph_sdf_full_create(sdf, sphere_phi, r2, &inertia_tensor); 

    return sdf;
}

srph_sdf * srph_sdf_torus_create(double r1, double r2){
    srph_sdf * sdf = (srph_sdf *) malloc(sizeof(srph_sdf));
    if (sdf == NULL){
        return NULL;
    }

    double * rs = (double *) malloc(2 * sizeof(double));
    if (rs == NULL){
        free(sdf);
        return NULL;
    }
    rs[0] = r1;
    rs[1] = r2;
    
    double xz = 1.0 / 8.0 * (4.0 * pow(r1, 2) + 3.0 * pow(r2, 2)); 
    double y = 1.0 / 4.0 * (4.0 * pow(r1, 2) + 5.0 * pow(r2, 2));
    srph::mat3_t inertia_tensor(
        xz, 0, 0,
        0,  y, 0,
        0,  0, xz
    );
    srph_sdf_full_create(sdf, torus_phi, rs, &inertia_tensor); 

    return sdf;
}
