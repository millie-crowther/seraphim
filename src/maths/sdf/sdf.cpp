#include "maths/sdf/sdf.h"

#include <iostream>

#include <stdlib.h>

#include "core/random.h"
#include "maths/sdf/primitive.h"
#include "physics/sphere.h"

#define VOLUME_SAMPLES 10000
#define SUPPORT_ALPHA 0.5

void srph_sdf_create(srph_sdf * sdf, srph_sdf_func phi, void * data){
    if (sdf == NULL){
        return;
    }

    sdf->_phi = phi;
    sdf->_data = data;
    
    sdf->_is_bound_valid = false;
    sdf->_is_com_valid = false;
    sdf->_is_inertia_tensor_valid = false;
    sdf->_is_convex = true; // TODO
    sdf->_volume = -1.0;

    srph_array_create(&sdf->sphere_approx, sizeof(srph_sphere));
}

srph::mat3_t srph_sdf_inertia_tensor(srph_sdf * sdf){
    if (!sdf->_is_inertia_tensor_valid){
        srph::mat3_t m;
        int hits = 0;
     
        srph_bound3 * b = srph_sdf_bound(sdf);
        srph_random rng;
        srph_random_default_seed(&rng);
    
        while (hits < VOLUME_SAMPLES){
            vec3 x = {
                srph_random_f64_range(&rng, b->lower[0], b->upper[0]),
                srph_random_f64_range(&rng, b->lower[1], b->upper[1]),
                srph_random_f64_range(&rng, b->lower[2], b->upper[2])
            };

            if (srph_sdf_contains(sdf, &x)){
                for (int i = 0; i < 3; i++){
                    for (int j = i; j < 3; j++){
                        vec3 r;
                        srph_vec3_subtract(&r, &x, srph_sdf_com(sdf));

                        double iij = -r.raw[i] * r.raw[j];

                        if (i == j){
                            iij += srph_vec3_dot(&r, &r);
                        }

                        m[i * 3 + j] += iij;
                        m[j * 3 + i] += iij;
                    }
                }     
            
   
                hits++;
            }
        }

        m /= (double) VOLUME_SAMPLES;
        sdf->_inertia_tensor = m;
        sdf->_is_inertia_tensor_valid = true;
    }

    return sdf->_inertia_tensor;
}

double srph_sdf_phi(srph_sdf * sdf, const vec3 * x){
    return sdf->_phi(sdf->_data, x);
}

vec3 srph_sdf_normal(srph_sdf * sdf, const vec3 * x){
    vec3 n;
    for (int i = 0; i < 3; i++){
        vec3 x1 = *x;
        x1.raw[i] += srph::constant::epsilon;

        vec3 x2 = *x;
        x2.raw[i] -= srph::constant::epsilon;
        
        n.raw[i] = srph_sdf_phi(sdf, &x1) - srph_sdf_phi(sdf, &x2);
    }
    
    srph_vec3_scale(&n, &n, 0.5 / srph::constant::epsilon);

    return n;
}

bool srph_sdf_contains(srph_sdf * sdf, const vec3 * x){
    return srph_sdf_phi(sdf, x) < 0.0;
}
    
double srph_sdf_project(srph_sdf * sdf, const vec3 * d){
    vec3 x;
    srph_vec3_normalise(&x, d);
    srph_vec3_scale(&x, &x, srph::constant::rho);

    while (true){
        double p = srph_sdf_phi(sdf, &x);
        
        // TODO: better quit criteria
        if (p > srph::constant::rho){
            return srph_vec3_length(&x) - p;
        }

        srph_vec3_scale(&x, &x, 2.0);
    }
}

double srph_sdf_volume(srph_sdf * sdf){
    if (sdf->_volume < 0.0){
        int hits = 0;
     
        srph_bound3 * b = srph_sdf_bound(sdf);
        srph_random rng;
        srph_random_default_seed(&rng);
    
        while (hits < VOLUME_SAMPLES){
            vec3 x = {
                srph_random_f64_range(&rng, b->lower[0], b->upper[0]),
                srph_random_f64_range(&rng, b->lower[1], b->upper[1]),
                srph_random_f64_range(&rng, b->lower[2], b->upper[2])
            };

            if (srph_sdf_contains(sdf, &x)){
                hits++;
            }
        }

        sdf->_volume = srph_bound3_volume(b) * (double) hits / (double) VOLUME_SAMPLES;
    }

    return sdf->_volume;
}

vec3 * srph_sdf_com(srph_sdf * sdf){
    if (!sdf->_is_com_valid){
        vec3 com = srph_vec3_zero;
        double hits = 0.0;
     
        srph_bound3 * b = srph_sdf_bound(sdf);
        srph_random rng;
        srph_random_default_seed(&rng);
    
        while (hits < VOLUME_SAMPLES){
            vec3 x = {
                srph_random_f64_range(&rng, b->lower[0], b->upper[0]),
                srph_random_f64_range(&rng, b->lower[1], b->upper[1]),
                srph_random_f64_range(&rng, b->lower[2], b->upper[2])
            };

            if (srph_sdf_contains(sdf, &x)){
                srph_vec3_add(&com, &com, &x);
                hits += 1.0;
            }
        }

        srph_vec3_scale(&com, &com, 1.0 / hits);
        sdf->_com = com;
        sdf->_is_com_valid = true;
    }

    return &sdf->_com;    
}

srph::mat3_t srph_sdf_jacobian(srph_sdf * sdf, const vec3 * x){
    srph::mat3_t j;

    for (int col = 0; col < 3; col++){
        vec3 x1 = *x;
        x1.raw[col] += srph::constant::epsilon;

        vec3 x2 = *x;
        x2.raw[col] -= srph::constant::epsilon;
        
        vec3 n1 = srph_sdf_normal(sdf, &x1);
        vec3 n2 = srph_sdf_normal(sdf, &x2);
        vec3 n;
        srph_vec3_subtract(&n, &n1, &n2);
        srph_vec3_scale(&n, &n, 0.5 / srph::constant::epsilon);

        for (int row = 0; row < 3; row++){
            j.set(row, col, n.raw[row]);
        } 
    }

    return j;        
}

srph_bound3 * srph_sdf_bound(srph_sdf * sdf){
    if (sdf == NULL){
        return NULL;
    } 

    if (!sdf->_is_bound_valid){
        vec3 a;
        for (int i = 0; i < 3; i++){
            srph_vec3_fill(&a, 0.0);

            a.raw[i] = -1.0;
            sdf->_bound.lower[i] = -srph_sdf_project(sdf, &a); 

            a.raw[i] = 1.0;
            sdf->_bound.upper[i] =  srph_sdf_project(sdf, &a); 
        }

        sdf->_is_bound_valid = true;
    }

    return &sdf->_bound;
}

void srph_sdf_destroy(srph_sdf * sdf){
    if (sdf != NULL){
        if (sdf->_data != NULL){
            free(sdf->_data);
        }

        srph_array_destroy(&sdf->sphere_approx);
        
        free(sdf);
    }
}
