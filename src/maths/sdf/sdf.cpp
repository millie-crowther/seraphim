#include "maths/sdf/sdf.h"

#include <stdlib.h>

#include "core/random.h"
#include "physics/sphere.h"

#include "core/constant.h"

#define VOLUME_SAMPLES 10000
#define SUPPORT_ALPHA 0.5

void srph_sdf_create(srph_sdf * sdf, srph_sdf_func phi, void * data){
    if (sdf == NULL){
        return;
    }

    sdf->_phi = phi;
    sdf->data = data;
    
    sdf->is_bound_valid = false;
    sdf->is_com_valid = false;
    sdf->is_inertia_tensor_valid = false;
    sdf->volume = -1.0;
}

mat3 * srph_sdf_inertia_tensor(srph_sdf * sdf){
    if (!sdf->is_inertia_tensor_valid){
        for (int i = 0; i < 9; i++){
            sdf->inertia_tensor.v[i] = 0.0;
        }

        int hits = 0;
     
        srph_bound3 * b = srph_sdf_bound(sdf);
        srph_random rng;
        srph_random_default_seed(&rng);
    
        while (hits < VOLUME_SAMPLES){
            vec3 x;
            x.x = srph_random_f64_range(&rng, b->lower[0], b->upper[0]);
            x.y = srph_random_f64_range(&rng, b->lower[1], b->upper[1]);
            x.z = srph_random_f64_range(&rng, b->lower[2], b->upper[2]);

            if (srph_sdf_contains(sdf, &x)){
                for (int i = 0; i < 3; i++){
                    for (int j = 0; j < 3; j++){
                        vec3 r;
                        vec3_subtract(&r, &x, srph_sdf_com(sdf));

                        double iij = -r.v[i] * r.v[j];

                        if (i == j){
                            iij += vec3_length_squared(&r);
                        }

                        sdf->inertia_tensor.v[j * 3 + i] += iij;
                    }
                }     

                hits++;
            }
        }

        mat3_multiply_f(&sdf->inertia_tensor, &sdf->inertia_tensor, 1.0 / VOLUME_SAMPLES);
        sdf->is_inertia_tensor_valid = true;
    }

    return &sdf->inertia_tensor;
}

double srph_sdf_phi(srph_sdf * sdf, const vec3 * x){
    return sdf->_phi(sdf->data, x);
}

vec3 srph_sdf_normal(srph_sdf * sdf, const vec3 * x){
    vec3 n;
    for (int i = 0; i < 3; i++){
        vec3 x1 = *x;
        x1.v[i] += srph::constant::epsilon;

        vec3 x2 = *x;
        x2.v[i] -= srph::constant::epsilon;
        
        n.v[i] = srph_sdf_phi(sdf, &x1) - srph_sdf_phi(sdf, &x2);
    }
    
    vec3_multiply_f(&n, &n, 0.5 / srph::constant::epsilon);

    return n;
}

bool srph_sdf_contains(srph_sdf * sdf, const vec3 * x){
    return srph_bound3_contains(&sdf->bound, x->v) && srph_sdf_phi(sdf, x) <= 0.0;
}
    
double srph_sdf_project(srph_sdf * sdf, const vec3 * d){
    vec3 x;
    vec3_normalize(&x, d);
    vec3_multiply_f(&x, &x, srph::constant::rho);

    while (true){
        double p = srph_sdf_phi(sdf, &x);
        
        // TODO: better quit criteria
        if (p > srph::constant::rho){
            return vec3_length(&x) - p;
        }

        vec3_multiply_f(&x, &x, 2.0);
    }
}

double srph_sdf_volume(srph_sdf * sdf){
    if (sdf->volume < 0.0){
        int hits = 0;
     
        srph_bound3 * b = srph_sdf_bound(sdf);
        srph_random rng;
        srph_random_default_seed(&rng);
    
        while (hits < VOLUME_SAMPLES){
            vec3 x;
            x.x = srph_random_f64_range(&rng, b->lower[0], b->upper[0]);
            x.y = srph_random_f64_range(&rng, b->lower[1], b->upper[1]);
            x.z = srph_random_f64_range(&rng, b->lower[2], b->upper[2]);

            if (srph_sdf_contains(sdf, &x)){
                hits++;
            }
        }

        sdf->volume = srph_bound3_volume(b) * (double) hits / (double) VOLUME_SAMPLES;
    }

    return sdf->volume;
}

vec3 * srph_sdf_com(srph_sdf * sdf){
    if (!sdf->is_com_valid){
        vec3 com = vec3_zero;
        double hits = 0.0;
     
        srph_bound3 * b = srph_sdf_bound(sdf);
        srph_random rng;
        srph_random_default_seed(&rng);
    
        while (hits < VOLUME_SAMPLES){
            vec3 x;
            x.x = srph_random_f64_range(&rng, b->lower[0], b->upper[0]);
            x.y = srph_random_f64_range(&rng, b->lower[1], b->upper[1]);
            x.z = srph_random_f64_range(&rng, b->lower[2], b->upper[2]);

            if (srph_sdf_contains(sdf, &x)){
                vec3_add(&com, &com, &x);
                hits += 1.0;
            }
        }

        vec3_multiply_f(&com, &com, 1.0 / hits);
        sdf->com = com;
        sdf->is_com_valid = true;
    }

    return &sdf->com;
}

srph_bound3 * srph_sdf_bound(srph_sdf * sdf){
    if (sdf == NULL){
        return NULL;
    } 

    if (!sdf->is_bound_valid){
        vec3 a;
        for (int i = 0; i < 3; i++){
            a = vec3_zero;

            a.v[i] = -1.0;
            sdf->bound.lower[i] = -srph_sdf_project(sdf, &a);

            a.v[i] = 1.0;
            sdf->bound.upper[i] =  srph_sdf_project(sdf, &a);
        }

        sdf->is_bound_valid = true;
    }

    return &sdf->bound;
}

void srph_sdf_destroy(srph_sdf * sdf){
    if (sdf != NULL){
        if (sdf->data != NULL){
            free(sdf->data);
        }
        
        free(sdf);
    }
}

double srph_sdf_discontinuity(srph_sdf *sdf, const vec3 *x) {
    vec3 ns[3];

    for (int axis = 0; axis < 3; axis++){
        vec3 x1 = *x;
        x1.v[axis] -= srph::constant::epsilon;
        vec3 x2 = *x;
        x2.v[axis] += srph::constant::epsilon;

        vec3 n1 = srph_sdf_normal(sdf, &x1);
        vec3 n2 = srph_sdf_normal(sdf, &x2);
        vec3_subtract(&ns[axis], &n2, &n1);

        vec3_divide_f(&ns[axis], &ns[axis], srph::constant::epsilon * 2);
    }

    return fabs(mat3_determinant((mat3 *) ns));
}

/*
void srph_sdf_add_sample(srph_sdf * sdf, const vec3 * x){
    assert(sdf != NULL);

    if (!srph_sdf_contains(sdf, x)){
        return;
    }

    for (uint32_t i = 0; i < sdf->vertices.size; i++){
        const vec3 * y = (vec3 *) srph_array_at(&sdf->vertices, i);
        if (srph_vec3_distance(x, y) < SAMPLE_DENSITY){
            return;
        }
    }

    *((vec3 *) srph_array_push_back(&sdf->vertices)) = *x;
} */
