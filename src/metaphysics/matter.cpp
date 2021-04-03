#include "metaphysics/matter.h"

#include <assert.h>


using namespace srph;

void srph_matter_init(
    srph_matter * m, srph_sdf * sdf, const srph_material * material,
    const vec3 * initial_position, bool is_uniform, bool is_static
){
    m->sdf = sdf;
    m->material = *material;
    m->is_uniform = is_uniform;
    m->is_static = is_static;
    m->is_rigid = true;

    // initialise fields for rigidbodies
    m->transform.position = *initial_position;
    m->transform.rotation = srph_quat_identity;
    m->v = srph_vec3_zero;
    m->omega = srph_vec3_zero;

    m->is_at_rest = false;
    m->is_inverse_inertia_tensor_valid = false;

    m->f = srph_vec3_zero;
    m->t = srph_vec3_zero;

    srph_array_init(&m->deformations);   
    m->origin = srph_matter_add_deformation(m, initial_position, srph_deform_type_control);
    m->origin->x0 = srph_vec3_zero;    

    vec3 com;
    srph_vec3_add(&com, srph_sdf_com(sdf), initial_position);
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
    srph_bound3_midpoint(b, s->c.raw);
    
    vec3 p = self->transform.position;
    srph_vec3_add(&s->c, &s->c, &p);
    
    vec3 r3;
    srph_bound3_radius(b, r3.raw);
    s->r = srph_vec3_length(&r3);

    vec3 v;
    srph_matter_linear_velocity(self, &v);
    s->r += srph_vec3_length(&v) * t;
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
            if (srph_vec3_distance(&x0, &deform->x0) < SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY){
                return NULL;
            }
        }
    }

    // create new deformation
    srph_deform * deform = (srph_deform *) malloc(sizeof(srph_deform)); 
    *deform = {
        .x0     = x0,
        .x      = *x,
        .v      = srph_vec3_zero,
        .p      = *x,
        .m      = 1.0, // TODO
        .type   = type,
    };

    // find average velocity
    for (size_t i = 0; i < self->deformations.size; i++){
        srph_vec3_add(&deform->v, &deform->v, &self->deformations.data[i]->v);
    }

    if (!srph_array_is_empty(&self->deformations)){
        srph_vec3_scale(&deform->v, &deform->v, 1.0 / (double) self->deformations.size);
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
    assert(m != NULL && tx != NULL && x != NULL);
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
    assert(m != NULL && xs != NULL);
    double dxs[16];
    srph_transform_matrix(&m->transform, dxs);

    for (int i = 0; i < 16; i++){
        xs[i] = (float) dxs[i];
    }
}

void srph_matter_to_global_position(const srph_matter * m, vec3 * tx, const vec3 * x){
    assert(m != NULL && tx != NULL && x != NULL);

    srph_transform_to_global_position(&m->transform, tx, x);
}

void srph_matter_to_global_direction(const srph_matter * m, const vec3 * position, vec3 * td, const vec3 * d){
    assert(m!= NULL && td != NULL && d != NULL);
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
    assert(self != NULL && v != NULL);

    if (self->is_rigid){
        *v = self->v;
    } else {
        *v = self->com->v;
        assert(false);
    }
}

void srph_matter_integrate_forces(srph_matter *self, double t, const vec3 *gravity) {
    assert(self != NULL && gravity != NULL && !self->is_static && !self->is_at_rest);

    double m = srph_matter_mass(self);

    // integrate force
    vec3 d;
    srph_vec3_scale(&d, &self->f, t / m);
    srph_vec3_add(&self->v, &self->v, &d);

    // integrate torque
    // TODO: inertia tensor here??
    srph_vec3_scale(&d, &self->t, t / m);
    srph_vec3_add(&self->omega, &self->omega, &d);

    // integrate linear velocity
    srph_vec3_scale(&d, &self->v, t);
    srph_transform_translate(&self->transform, &d);

    // integrate angular velocity
    srph_vec3_scale(&d, &self->omega, t);
    srph_quat q;
    srph_quat_euler_angles(&q, &d);
    srph_matter_rotate(self, &q);

    // reset forces
    srph_vec3_scale(&self->f, gravity, m);
    self->t = srph_vec3_zero;
}

static void offset_from_centre_of_mass(srph_matter * self, vec3 * r, const vec3 *x){
    assert (self != NULL && r != NULL);

    vec3 com;

    if (self->is_rigid){
        srph_matter_to_global_position(self, &com, srph_sdf_com(self->sdf));
    } else {
        com = self->com->x;
        assert(false);
    }

    srph_vec3_subtract(r, x, &com);
}

void srph_matter_velocity(srph_matter *self, const vec3 *x, vec3 *v) {
    assert(self != NULL && x != NULL && v != NULL);

    if (self->is_rigid){
        vec3 r;
        offset_from_centre_of_mass(self, &r, x);
        srph_vec3_cross(v, &self->omega, &r);
        srph_vec3_add(v, v, &self->v);
    } else {
        assert(false);
    }
}

void srph_matter_material(srph_matter *self, srph_material *mat) {
    assert(self != NULL && mat != NULL);
    *mat = self->material;
}

static srph_mat3 * inverse_inertia_tensor(srph_matter *self){
    if (!self->is_inverse_inertia_tensor_valid){
        // rotate
        srph_mat3 r, rt;
        srph_quat_to_matrix(&self->transform.rotation, &r);
        srph_matrix_transpose(&r, &rt);

        srph_mat3 * i = srph_sdf_inertia_tensor(self->sdf);
        srph_matrix_multiply(&self->inverse_inertia_tensor, i, &rt);
        srph_matrix_multiply(&self->inverse_inertia_tensor, &r, &self->inverse_inertia_tensor);

        // invert
        srph_matrix_inverse(&self->inverse_inertia_tensor, &self->inverse_inertia_tensor);
        self->is_inverse_inertia_tensor_valid = true;
    }

    return &self->inverse_inertia_tensor;
}

void srph_matter_apply_impulse_at(srph_matter *self, const vec3 *x, const vec3 *j) {
    vec3 d;
    srph_vec3_scale(&d, j, 1.0 / srph_matter_mass(self));

    vec3 r, rxj;
    offset_from_centre_of_mass(self, &r, x);

    srph_mat3 * i = inverse_inertia_tensor(self);
    srph_vec3_cross(&rxj, &r, j);

    srph_matrix_apply(i, &rxj, &rxj);
    srph_vec3_add(&self->omega, &self->omega, &rxj);
}

double srph_matter_inverse_angular_mass(srph_matter *self, vec3 *x, vec3 *n) {
    vec3 r, rn, irn;
    offset_from_centre_of_mass(self, &r, x);
    srph_vec3_cross(&rn, &r, n);

    srph_mat3 * i = inverse_inertia_tensor(self);
    srph_matrix_apply(i, &rn, &irn);
    return srph_vec3_dot(&rn, &irn);
}

void srph_matter_rotate(srph_matter *self, srph_quat *q) {
    srph_transform_rotate(&self->transform, q);
    self->is_inverse_inertia_tensor_valid = false;
}

bool srph_matter_is_at_rest(srph_matter *self) {
    return self->is_at_rest || self->is_static;
}
