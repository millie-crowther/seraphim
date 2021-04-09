#include "metaphysics/matter.h"

#include <assert.h>
#include <math.h>


void srph_matter_init(
    srph_matter * m, srph_sdf * sdf, const srph_material * material,
    const vec3 * initial_position, bool is_uniform, bool is_static
){
    m->sdf = sdf;
    m->material = *material;
    m->is_uniform = is_uniform;
    m->is_static = is_static;
    m->is_rigid = true;

    // initialise fields for rigid bodies
    m->transform.position = *initial_position;
    m->transform.rotation = quat_identity;
    m->v = vec3_zero;
    m->omega = vec3_zero;

    m->is_at_rest = false;
    m->is_inverse_inertia_tensor_valid = false;

    m->f = vec3_zero;
    m->t = vec3_zero;

    srph_array_init(&m->deformations);   
    m->origin = srph_matter_add_deformation(m, initial_position, srph_deform_type_control);
    m->origin->x0 = vec3_zero;

    vec3 com;
    vec3_add(&com, srph_sdf_com(sdf), initial_position);
    m->com = srph_matter_add_deformation(m, &com, srph_deform_type_control);
}

void srph_matter_destroy(srph_matter * m){
    while (!srph_array_is_empty(&m->deformations)){
        free(*m->deformations.last);
        srph_array_pop_back(&m->deformations);
    }
    srph_array_clear(&m->deformations);
}

bool srph_matter_is_inert(srph_matter * m){
    return false;
}

//double srph_matter::get_inverse_angular_mass(const vec3_t & r_global, const vec3_t & n){
//    auto r = r_global - transform.to_global_space(get_centre_of_mass());
//    auto rn = vec::cross(r, n);
//
//    return vec::dot(rn, *get_inv_tf_i() * rn);
//}
//
//void srph_matter::apply_impulse_at(const vec3_t & j, const vec3_t & r_global){
//    v += j / srph_matter_mass(this);
//    auto r = r_global - transform.to_global_space(get_centre_of_mass());
//    omega += *get_inv_tf_i() * vec::cross(r, j);
//}

double srph_matter_average_density(srph_matter *self){
    if (self->is_uniform){
        return self->material.density;
    }

    assert(false);

    return 0.0;
}

double srph_matter_mass(srph_matter * self){
    return srph_matter_average_density(self) * srph_sdf_volume(self->sdf);
}

//void srph_matter::rotate(const quat_t & q){
//    transform.rotate(q);
//    _is_inv_inertia_tensor_valid = false;
//}

//mat3_t * srph_matter::get_inv_tf_i(){
//    if (!_is_inv_inertia_tensor_valid){
//        inv_tf_i = *get_i();
//
//        // rotate
//        auto r = transform.get_rotation().to_matrix();
//        inv_tf_i = r * i * mat::transpose(r);
//
//
//        // invert
//        inv_tf_i = mat::inverse(inv_tf_i);
//        _is_inv_inertia_tensor_valid = true;
//    }
//
//    return &inv_tf_i;
//}

mat3 * inverse_inertia_tensor(srph_matter *self){
    if (!self->is_inverse_inertia_tensor_valid){
        // rotate
        mat3 r, rt;
        mat3_rotation_quat(&r, &self->transform.rotation);
        mat3_transpose(&rt, &r);

        mat3 i;
        mat3_multiply_f(&i, srph_sdf_inertia_tensor(self->sdf), srph_matter_mass(self));

        mat3 ri;
        mat3_multiply(&ri, &i, &rt);
        mat3_multiply(&ri, &r, &ri);

        // invert
        mat3_inverse(&self->inverse_inertia_tensor, &ri);
        self->is_inverse_inertia_tensor_valid = true;

        for (int j = 0; j < 9; j++){
            assert(isfinite(self->inverse_inertia_tensor.v[j]));
        }
    }

    return &self->inverse_inertia_tensor;
}

//
//mat3_t * srph_matter::get_i(){
//    if (!is_inertia_tensor_valid){
//        if (is_uniform){
//            i = srph_sdf_inertia_tensor(sdf) * srph_matter_mass(this);
//        } else {
//            throw std::runtime_error("Error: non uniform substances not yet supported.");
//        }
//        is_inertia_tensor_valid = true;
//    }
//
//    return &i;
//}
//

void srph_matter_sphere_bound(const srph_matter * self, double t, srph_sphere * s){
    srph_bound3 * b = srph_sdf_bound(self->sdf);
    srph_bound3_midpoint(b, s->c.v);
    
    vec3 p = self->transform.position;
    vec3_add(&s->c, &s->c, &p);
    
    vec3 r3;
    srph_bound3_radius(b, r3.v);
    s->r = vec3_length(&r3);

    vec3 v;
    srph_matter_linear_velocity(self, &v);
    s->r += vec3_length(&v) * t;
}

srph_deform * srph_matter_add_deformation(srph_matter * self , const vec3 * x, srph_deform_type type){
    // transform position into local space
    vec3 x0;
    srph_matter_to_local_position(self, &x0, x);

    // check if new deformation is inside surface and not too close to any other deformations
    if (type == srph_deform_type_collision){
        if (!srph_sdf_contains(self->sdf, &x0)){
            return NULL;
        }        

        for (size_t i = 0; i < self->deformations.size; i++){
            srph_deform * deform = self->deformations.data[i];
            if (vec3_distance(&x0, &deform->x0) < SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY){
                return NULL;
            }
        }
    }

    // create new deformation
    srph_deform * deform = (srph_deform *) malloc(sizeof(srph_deform)); 
    *deform = {
        .x0     = x0,
        .x      = *x,
        .v      = vec3_zero,
        .p      = *x,
        .m      = 1.0, // TODO
        .type   = type,
    };

    // find average velocity
    for (size_t i = 0; i < self->deformations.size; i++){
        vec3_add(&deform->v, &deform->v, &self->deformations.data[i]->v);
    }

    if (!srph_array_is_empty(&self->deformations)){
        vec3_multiply_f(&deform->v, &deform->v, 1.0 / (double) self->deformations.size);
    }

    // add to list of deformations
    srph_array_push_back(&self->deformations);
    *self->deformations.last = deform;

    return deform;
}

//void srph_matter_update_vertices(srph_matter * m, double t){
//    assert(m != NULL);

    // update velocities using newton's second law
//    vec3 a, r;
//
//    for (uint32_t i = 0; i < m->deformations.size; i++){
//        srph_deform * deform = m->deformations.data[i];
//        srph_vec3_subtract(&r, &deform->x, &m->com->x);
//        srph_vec3_cross(&a, &m->t, &r);
//        srph_vec3_add(&a, &a, &m->f);
//
//        srph_vec3_scale(&a, &a, t / deform->m);
//        srph_vec3_add(&deform->v, &deform->v, &a);
//    }

    // TODO: velocity dampening

    // extrapolate next position
//    for (uint32_t i = 0; i < m->deformations.size; i++){
//        srph_deform * deform = m->deformations.data[i];
//        srph_vec3_scale(&deform->p, &deform->v, t);
//        srph_vec3_add(&deform->p, &deform->p, &deform->x);
//    }
//}

void srph_matter_to_local_position(srph_matter *m, vec3 * tx, const vec3 * x){
    srph_transform_to_local_position(&m->transform, tx, x);
}

//void srph_matter_extrapolate_next_position_and_velocity(srph_matter * m, double t){
//    if (m->is_rigid){
//
//    } else {
//        for (uint32_t i = 0; i < m->deformations.size; i++) {
//            srph_deform *deform = m->deformations.data[i];
//
//            srph_vec3_subtract(&deform->v, &deform->p, &deform->x);
//            srph_vec3_scale(&deform->v, &deform->v, 1.0 / t);
//            deform->x = deform->p;
//        }
//    }
//}

void srph_matter_transformation_matrix(srph_matter * m, float * xs){
    mat4 dxs;
    srph_transform_matrix(&m->transform, &dxs);

    for (int i = 0; i < 16; i++){
        xs[i] = (float) dxs.v[i];
    }
}

void srph_matter_to_global_position(const srph_matter * m, vec3 * tx, const vec3 * x){
    srph_transform_to_global_position(&m->transform, tx, x);
}

void srph_matter_to_global_direction(const srph_matter * m, const vec3 * position, vec3 * td, const vec3 * d){
    srph_transform_to_global_direction(&m->transform, td, d);
}

//void srph_matter_resolve_internal_constraints(const srph_matter * m){
//    assert(m != NULL);
//
//    if (m->is_rigid){
//        return;
//    }
//
//    // resolve distance constraint
//    for (size_t i = 0; i < m->deformations.size; i++){
//        srph_deform * a = m->deformations.data[i];
//
//        for (size_t j = i + 1; j < m->deformations.size; j++){
//            srph_deform * b = m->deformations.data[j];
//
//            vec3 ab;
//            srph_vec3_subtract(&ab, &b->p, &a->p);
//
//            double d = srph_vec3_length(&ab) - srph_vec3_distance(&a->x0, &b->x0);
//
//            if (d == 0.0){
//                continue;
//            }
//
//            srph_vec3_normalise(&ab, &ab);
//            srph_vec3_scale(&ab, &ab, d / 2);
//
//            srph_vec3_add(&a->p, &a->p, &ab);
//            srph_vec3_subtract(&b->p, &b->p, &ab);
//        }
//    }
//}

void srph_matter_linear_velocity(const srph_matter *self, vec3 *v) {
    if (self->is_rigid){
        *v = self->v;
    } else {
        *v = self->com->v;
        assert(false);
    }
}

void srph_matter_integrate_forces(srph_matter *self, double t, const vec3 *gravity) {
    assert(!self->is_static && !self->is_at_rest);

    double m = srph_matter_mass(self);

    // integrate force
    vec3 d;
    vec3_multiply_f(&d, &self->f, t / m);
    vec3_add(&self->v, &self->v, &d);

    // integrate torque
    // TODO: inertia tensor here??
    vec3_multiply_f(&d, &self->t, t / m);
    vec3_add(&self->omega, &self->omega, &d);

    // integrate linear velocity
    vec3_multiply_f(&d, &self->v, t);
    srph_transform_translate(&self->transform, &d);

    // integrate angular velocity
    vec3_multiply_f(&d, &self->omega, t);
    quat q;
    quat_from_euler_angles(&q, &self->omega);
    assert(isfinite(q.x));
    assert(isfinite(self->omega.x));
    srph_matter_rotate(self, &q);

    // reset forces
    vec3_multiply_f(&self->f, gravity, m);
    self->t = vec3_zero;
}

static void offset_from_centre_of_mass(srph_matter * self, vec3 * r, const vec3 *x){
    vec3 com;

    if (self->is_rigid){
        srph_matter_to_global_position(self, &com, srph_sdf_com(self->sdf));
    } else {
        com = self->com->x;
        assert(false);
    }

    vec3_subtract(r, x, &com);
}

void srph_matter_velocity(srph_matter *self, const vec3 *x, vec3 *v) {
    if (self->is_rigid){
        vec3 r;
        offset_from_centre_of_mass(self, &r, x);
        vec3_cross(v, &self->omega, &r);
        vec3_add(v, v, &self->v);
    } else {
        assert(false);
    }
}

void srph_matter_material(srph_matter *self, srph_material *mat) {
    *mat = self->material;
}

void srph_matter_apply_impulse(srph_matter *self, const vec3 *x, const vec3 *n, double j) {
    double m = srph_matter_mass(self);
    assert(m > 0);
//    assert(n->y > 0);
    vec3 dv;
    vec3_multiply_f(&dv, n, j / m);
    vec3_add(&self->v, &self->v, &dv);

    vec3 r, dw;
    offset_from_centre_of_mass(self, &r, x);
    vec3_cross(&dw, &r, n);

    mat3 * i = inverse_inertia_tensor(self);
    vec3_multiply_mat3(&dw, &dw, i);
    vec3_multiply_f(&dw, &dw, j);
    vec3_add(&self->omega, &self->omega, &dw);
}

double srph_matter_inverse_angular_mass(srph_matter *self, vec3 *x, vec3 *n) {
    vec3 r, rn, rnr;
    offset_from_centre_of_mass(self, &r, x);
    vec3_cross(&rn, &r, n);
    vec3_cross(&rnr, &rn, &r);

    mat3 * i = inverse_inertia_tensor(self);
    vec3_multiply_mat3(&rnr, &rnr, i);
    double im = vec3_dot(&rnr, n);
    assert(isfinite(im));
    return im;
}

void srph_matter_rotate(srph_matter *self, quat *q) {
    srph_transform_rotate(&self->transform, q);
    assert(isfinite(q->x));
    self->is_inverse_inertia_tensor_valid = false;
}

bool srph_matter_is_at_rest(srph_matter *self) {
    return self->is_at_rest || self->is_static;
}
