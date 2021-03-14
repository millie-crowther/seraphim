#include "metaphysics/matter.h"

static void update_vertices(srph_matter * m){
    if (m->_vertices.size >= m->sdf->vertices.size){
        return;
    }

    // compute average translation and velocity for inserting new vertices
    vec3 d = srph_vec3_zero;
    vec3 v = srph_vec3_zero;
    for (uint32_t i = 0; i < m->_vertices.size; i++){
        srph_vertex * vertex = (srph_vertex *) srph_array_at(&m->_vertices, i);
        srph_vec3_add(&d, &d, &vertex->x);
        srph_vec3_subtract(&d, &d, vertex->_x_key);

        srph_vec3_add(&v, &v, &vertex->v);
    }

    if (!srph_array_is_empty(&m->_vertices)){
        srph_vec3_scale(&d, &d, 1.0 / (double) m->_vertices.size);
        srph_vec3_scale(&v, &v, 1.0 / (double) m->_vertices.size);
    }

    while (m->_vertices.size < m->sdf->vertices.size){
        vec3 * x_sdf = (vec3 *) srph_array_at(&m->sdf->vertices, m->_vertices.size);

        srph_vertex * vertex = (srph_vertex *) srph_array_push_back(&m->_vertices);
        vertex->_x_key = x_sdf;
        vertex->w = 1.0; // TODO
        vertex->v = v;
        srph_vec3_fill(&vertex->p, 0.0);
        srph_vec3_add(&vertex->x, x_sdf, &d);
    }
}

using namespace srph;



void srph_matter_init(
    srph_matter * m, srph_sdf * sdf, const srph_material * material, 
    const vec3 * x, bool is_uniform
){
    m->sdf = sdf;
    m->material = *material;
    m->is_uniform = is_uniform;
    
    m->transform.set_position(srph::vec3_t(x->x, x->y, x->z));

    m->omega = vec3_t(0.01, 0.01, 0.01);

    m->_is_mass_calculated = false;
    m->_is_inertia_tensor_valid = false;
    m->_is_inv_inertia_tensor_valid = false;

    srph_vec3_fill(&m->f, 0.0);
    srph_vec3_fill(&m->t, 0.0);
    
    srph_array_create(&m->_vertices, sizeof(srph_vertex));
    
    update_vertices(m); // TODO: remove
}

void srph_matter_destroy(srph_matter * m){
    srph_array_destroy(&m->_vertices);
}

quat_t srph_matter::get_rotation() const {
    return transform.get_rotation();
}

vec3_t srph_matter::get_position() const {
    return transform.get_position();
}

bool srph_matter::is_inert(){
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
