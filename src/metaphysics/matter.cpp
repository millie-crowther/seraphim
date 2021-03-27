#include "metaphysics/matter.h"

#include <assert.h>

using namespace srph;

void srph_matter_init(
    srph_matter * m, srph_sdf * sdf, const srph_material * material, 
    const vec3 * initial_position, bool is_uniform
){
    m->sdf = sdf;
    m->material = *material;
    m->is_uniform = is_uniform;
    
    m->transform.set_position(srph::vec3_t(
        initial_position->x, initial_position->y, initial_position->z
    ));

    m->omega = vec3_t(0.01, 0.01, 0.01);

    m->is_at_rest = false;
    m->_is_mass_calculated = false;
    m->_is_inertia_tensor_valid = false;
    m->_is_inv_inertia_tensor_valid = false;

    m->f = srph_vec3_zero;
    m->t = srph_vec3_zero;

    srph_array_init(&m->deformations);   
    vec3 com;
    srph_vec3_add(&com, srph_sdf_com(sdf), initial_position);
    m->com = srph_matter_add_deformation(m, &com, srph_deform_type_control);
    m->origin = srph_matter_add_deformation(m, initial_position, srph_deform_type_control);
}

void srph_matter_destroy(srph_matter * m){
    while (!srph_array_is_empty(&m->deformations)){
        free(m->deformations.last);
        srph_array_pop_back(&m->deformations);
    }
    srph_array_clear(&m->deformations);
}

void srph_matter_push_internal_constraints(srph_matter * m, srph_constraint_array * a){
    assert(m != NULL && a != NULL);
    
    for (size_t i = 0; i < m->deformations.size; i++){
        for (size_t j = i + 1; j < m->deformations.size; j++){
            srph_array_push_back(a);
            srph_constraint_distance(
                a->last, m->deformations.data[i], m->deformations.data[j], 1.0
            );
        }
    }
}

quat_t srph_matter::get_rotation() const {
    return transform.get_rotation();
}

vec3_t srph_matter::get_position() const {
    return transform.get_position();
}

bool srph_matter_is_inert(srph_matter * m){
    return false;
}

srph_material srph_matter::get_material(const vec3 * x){
    return material;
}

void srph_matter_bound(const srph_matter * m, srph_bound3 * b){
    if (m == NULL || b == NULL){
        return;
    }

    srph_bound3_create(b);

    srph_bound3 * sdf_bound = srph_sdf_bound(m->sdf);
    for (int i = 0; i < 8; i++){  
        vec3 x1;
        srph_bound3_vertex(sdf_bound, i, x1.raw);
    
        vec3_t x(x1.x, x1.y, x1.z);
        x = m->transform.to_global_space(x);
    
        x1 = { x[0], x[1], x[2] };

        srph_bound3_capture(b, x1.raw);
    }
}

srph_bound3 srph_matter::get_moving_bound(double t) const {
    srph_bound3 bound;
    srph_matter_bound(this, &bound);

    for (int i = 0; i < 3; i++){
        if (v[i] >= 0){
            bound.upper[i] += v[i] * t;
        } else {
            bound.lower[i] += v[i] * t;
        }
    }      

    // TODO: include rotation    

    return bound;
}

double srph_matter::get_inverse_angular_mass(const vec3_t & r_global, const vec3_t & n){
    auto r = r_global - transform.to_global_space(get_centre_of_mass()); 
    auto rn = vec::cross(r, n);

    return vec::dot(rn, *get_inv_tf_i() * rn);
}

f32mat4_t srph_matter::get_matrix(){
    return transform.get_matrix();
}

void srph_matter::apply_impulse_at(const vec3_t & j, const vec3_t & r_global){
    v += j / srph_matter_mass(this);
    auto r = r_global - transform.to_global_space(get_centre_of_mass()); 
    omega += *get_inv_tf_i() * vec::cross(r, j);
}

void srph_matter::calculate_centre_of_mass(){
    _is_mass_calculated = true;
    throw std::runtime_error("Error: autocalc of centre of mass not yet implemented.");
}

vec3_t srph_matter::get_centre_of_mass(){
    if (is_uniform){
        vec3 * c = srph_sdf_com(sdf);
        return vec3_t(c->x, c->y, c->z);
    }

    if (!_is_mass_calculated){
        calculate_centre_of_mass();
    }

    return centre_of_mass;
}

double srph_matter::get_average_density(){
    if (is_uniform){
        return get_material(&srph_vec3_zero).density;
    }

    if (!_is_mass_calculated){
        calculate_centre_of_mass();
    }

    return average_density;
}

double srph_matter_mass(srph_matter * m){
    return m->get_average_density() * srph_sdf_volume(m->sdf);
}

void srph_matter::translate(const vec3_t & x){
    transform.translate(x);
}

void srph_matter::rotate(const quat_t & q){
    transform.rotate(q);
    _is_inv_inertia_tensor_valid = false;
}

void srph_matter::reset_acceleration() {
    a = vec3_t(0.0, -9.8, 0.0);
}

void srph_matter::physics_tick(double t){
    // update position
    transform.translate((a * 0.5 * t + v) * t);
    
    // update rotation
    transform.rotate(quat_t::euler_angles(omega * t));

    // integrate accelerations into velocities
    v += a * t;
    
    if (transform.get_position()[1] < -90.0){
        transform.set_position(vec3_t(0.0, -100.0, 0.0));
        v = vec3_t();
        omega = vec3_t();
        a = vec3_t();
    }    
}

mat3_t * srph_matter::get_inv_tf_i(){
    if (!_is_inv_inertia_tensor_valid){
        inv_tf_i = *get_i();        

        // rotate
        auto r = transform.get_rotation().to_matrix();
        inv_tf_i = r * i * mat::transpose(r);
    
    
        // invert
        inv_tf_i = mat::inverse(inv_tf_i);
        _is_inv_inertia_tensor_valid = true;
    }

    return &inv_tf_i;
}

mat3_t * srph_matter::get_i(){
    if (!_is_inertia_tensor_valid){
        if (is_uniform){
            i = srph_sdf_inertia_tensor(sdf) * srph_matter_mass(this);
        } else {
            throw std::runtime_error("Error: non uniform substances not yet supported.");
        }
        _is_inertia_tensor_valid = true;
    }
    
    return &i;
}

vec3_t srph_matter::get_velocity(const vec3_t & x){
    vec3_t x1 = x - transform.to_global_space(get_centre_of_mass());
    return v + vec::cross(omega, x1);
}

vec3_t srph_matter::to_local_space(const vec3_t & x) const {
    return transform.to_local_space(x);
}

void srph_matter_sphere_bound(const srph_matter * m, double t, srph_sphere * s){
    srph_bound3 * b = srph_sdf_bound(m->sdf);
    srph_bound3_midpoint(b, s->c.raw);
    
    srph::vec3_t p1 = m->transform.get_position();
    vec3 p = { p1[0], p1[1], p1[2] };
    srph_vec3_add(&s->c, &s->c, &p);
    
    vec3 r3;
    srph_bound3_radius(b, r3.raw);
    s->r = srph_vec3_length(&r3);

    vec3 v1 = { m->v[0], m->v[1], m->v[2] };
    s->r += srph_vec3_length(&v1) * t;
}

srph_deform * srph_matter_add_deformation(srph_matter * m , const vec3 * x, srph_deform_type type){
    // compute average translation and velocity for inserting new vertices
    vec3 d = srph_vec3_zero;
    vec3 v = srph_vec3_zero;
    for (uint32_t i = 0; i < m->deformations.size; i++){
        srph_deform * deform = m->deformations.data[i];
        srph_vec3_add(&d, &d, &deform->x);
        srph_vec3_subtract(&d, &d, &deform->x0);

        srph_vec3_add(&v, &v, &deform->v);
    }

    if (!srph_array_is_empty(&m->deformations)){
        srph_vec3_scale(&d, &d, 1.0 / (double) m->deformations.size);
        srph_vec3_scale(&v, &v, 1.0 / (double) m->deformations.size);
    }

    // check if new deformation is inside surface and not too close to any other deformations
    vec3 x0;
    srph_vec3_subtract(&x0, x, &d);
    if (type == srph_deform_type_collision){
        if (!srph_sdf_contains(m->sdf, &x0)){
            return NULL;
        }        

        for (size_t i = 0; i < m->deformations.size; i++){
            srph_deform * d = m->deformations.data[i];
            if (srph_vec3_distance(&x0, &d->x0) < SERAPHIM_DEFORM_MAX_SAMPLE_DENSITY){
                return NULL;
            }
        }
    } 

    // create new deformation
    srph_deform * deform = (srph_deform *) malloc(sizeof(srph_deform)); 
    *deform = {
        .x0   = x0,
        .x    = *x,
        .v    = v,
        .p    = *x,
        .m    = 1.0, // TODO
        .type = type,
    };
    
    srph_array_push_back(&m->deformations);
    *m->deformations.last = deform;    

    return deform;
}

void srph_matter_update_vertices(srph_matter * m, double t){
    assert(m != NULL);

    // update velocities using newton's second law
    vec3 a, r;
    vec3 * com = srph_matter_com(m);

    for (uint32_t i = 0; i < m->deformations.size; i++){
        srph_deform * deform = m->deformations.data[i];
        srph_vec3_subtract(&r, &deform->x, com);
        srph_vec3_cross(&a, &m->t, &r);
        srph_vec3_add(&a, &a, &m->f);

        srph_vec3_scale(&a, &a, t / deform->m);
        srph_vec3_add(&deform->v, &deform->v, &a);
    }

    // TODO: velocity dampening

    // extrapolate next position
    for (uint32_t i = 0; i < m->deformations.size; i++){
        srph_deform * deform = m->deformations.data[i];
        srph_vec3_scale(&deform->p, &deform->v, t);
        srph_vec3_add(&deform->p, &deform->p, &deform->x);
    }
}

void srph_matter_to_local_space(const srph_matter * m, vec3 * tx, const vec3 * x){
    srph_vec3_subtract(tx, x, &m->origin->x);

    srph_quat q;
    srph_matter_rotation(m, &q);
    srph_quat_inverse(&q, &q);
    srph_quat_rotate(&q, tx, tx);
}

void srph_matter_update_velocities(srph_matter * m, double t){
    // update next position and velocity
    for (uint32_t i = 0; i < m->deformations.size; i++){
        srph_deform * deform = m->deformations.data[i];
        srph_vec3_subtract(&deform->v, &deform->p, &deform->x);
        srph_vec3_scale(&deform->v, &deform->v, 1.0 / t);
        deform->v = deform->p;
    }

    // TODO: friction

    // TODO: restitution
}

vec3 * srph_matter_com(srph_matter * m){
    return &m->com->x;
}

void srph_matter_rotation(const srph_matter * m, srph_quat * q){
    assert(m != NULL && q != NULL);

    if (srph_array_is_empty(&m->deformations)){
        *q = srph_quat_identity;
        return;
    }

    vec3 * o = &m->origin->x;   
    *q = { 0.0, 0.0, 0.0, 0.0 };

    for (size_t i = 0; i < m->deformations.size; i++){
        srph_deform * d = m->deformations.data[i];

        vec3 x;
        srph_vec3_subtract(&x, &d->x, o);

        srph_quat qi;
        srph_quat_rotate_to(&qi, &d->x0, &x);

        for (int i = 0; i < 4; i++){
            q->raw[i] += qi.raw[i];
        }
    }

    for (int i = 0; i < 4; i++){
        q->raw[i] /= m->deformations.size; 
    }
}

void srph_matter_transformation(const srph_matter * matter, float * xs){
    assert(matter != NULL && xs != NULL);

    vec3 * o = &matter->origin->x;   

    srph_quat q_total;
    srph_matter_rotation(matter, &q_total);

    double m[9];
    srph_quat_to_matrix(&q_total, m);

    xs[0]  = (float) m[0]; 
    xs[1]  = (float) m[1]; 
    xs[2]  = (float) m[2]; 
    xs[3]  = (float) o->x,
    
    xs[4]  = (float) m[3]; 
    xs[5]  = (float) m[4]; 
    xs[6]  = (float) m[5]; 
    xs[7]  = (float) o->y,
    
    xs[8]  = (float) m[6]; 
    xs[9]  = (float) m[7]; 
    xs[10] = (float) m[8]; 
    xs[11] = (float) o->z,

    xs[12] = 0.0;
    xs[13] = 0.0;
    xs[14] = 0.0;
    xs[15] = 1.0;
}

void srph_matter_normal(const srph_matter * m, const vec3 * x, vec3 * n){
    assert(m != NULL && x != NULL && n != NULL);

    vec3 x_local;
    srph_matter_to_local_space(m, &x_local, x);
   
    *n = srph_sdf_normal(m->sdf, &x_local);
 
    srph_quat q;
    srph_matter_rotation(m, &q);
    srph_quat_rotate(&q, n, n);    
}
