#include "maths/sdf/primitive.h"

#include <math.h>
#include <stdlib.h>

double sphere_phi(void * data, const vec3 * x){
    double r = *((double *) data);
    return vec3_length(x) - r;
}

double torus_phi(void * data, const vec3 * x){
    double * rs = (double *) data;
    vec2 xy;
    xy.x = hypot(x->x, x->z) - rs[0];
    xy.y = x->y;
    return hypot(xy.x, xy.y) - rs[1];
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

    srph_sdf_create(sdf, sphere_phi, r2); 

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
    
    srph_sdf_create(sdf, torus_phi, rs); 

    return sdf;
}
