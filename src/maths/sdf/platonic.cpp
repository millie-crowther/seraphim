#include "maths/sdf/platonic.h"

static double cuboid_phi(void * data, const vec3 * x){
    vec3 * r = (vec3 *) data;
    vec3 x1 = *x;

    srph_vec3_abs(&x1, &x1);

    vec3 q;
    srph_vec3_subtract(&q, &x1, r);
    
    double m = q.x;
    for (int i = 0; i < 3; i++){
        m = fmax(m, q.raw[i]);
    }
    srph_vec3_max_scalar(&q, &q, 0.0);

    return srph_vec3_length(&q) + fmin(m, 0.0);
}

static double octahedron_phi(void * data, const vec3 * x){
    double e = *((double *) data);
    double s = e / sqrt(2);
    vec3 p = *x;
    srph_vec3_abs(&p, &p);    

    float m = p.x + p.y + p.z - s;
    
    vec3 q;
    if (3.0 * p.x < m ){
        q = { p.x, p.y, p.z };
    } else if (3.0 * p.y < m){
        q = { p.y, p.z, p.x };
    } else if (3.0 * p.z < m){
        q = { p.z, p.x, p.y };
    } else {
        return m * 0.57735027;
    }

    float k = 0.5 * (q.z - q.y + s);
    k = fmax(k, 0.0);
    k = fmin(k, s);

    vec3 r = { q.x, q.y - s + k, q.z - k };
    return srph_vec3_length(&r);  
}

void srph_sdf_cuboid_create(srph_sdf_cuboid * cuboid, const vec3 * r){
    if (cuboid == NULL){
        return;
    }

    cuboid->_r = *r;

    vec3 r2;
    srph_vec3_multiply(&r2, r, r);
    srph::mat3_t i(
        r2.y + r2.z, 0.0,         0.0,
        0.0,         r2.x + r2.z, 0.0,
        0.0,         0.0,         r2.x + r2.y
    );
    i /= 12.0;
    
    srph_sdf_full_create(&cuboid->sdf, cuboid_phi, (void *) &cuboid->_r, &i); 
}

void srph_sdf_octahedron_create(srph_sdf_octahedron * octa, double e){
    if (octa == NULL){
        return;
    }

    octa->_e = e;

    srph::mat3_t i = srph::mat3_t::diagonal(0.1 * pow(e, 2));
    srph_sdf_full_create(&octa->sdf, octahedron_phi, (void *) &octa->_e, &i);
}
