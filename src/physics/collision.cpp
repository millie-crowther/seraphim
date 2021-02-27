#include "physics/collision.h"

#include <float.h>

#include "maths/matrix.h"
#include "maths/vector.h"

#include "maths/optimise.h"

#include <map>
#include <queue>

static double intersection_func(void * data, const vec3 * x){
    srph::collision_t * collision = (srph::collision_t *) data;
    srph_matter * a = collision->a;
    srph_matter * b = collision->b;
    
    vec3 xa, xb;
    srph_transform_to_local_space(&a->transform, &xa, x);
    srph_transform_to_local_space(&b->transform, &xb, x);

    double phi_a = srph_sdf_phi(a->sdf, &xa);
    double phi_b = srph_sdf_phi(b->sdf, &xb);

    return std::max(phi_a, phi_b);
}

static double time_to_collision_func(void * data, const vec3 * x){
    srph::collision_t * collision = (srph::collision_t *) data;
    srph_matter * a = collision->a;
    srph_matter * b = collision->b;
    
    vec3 xa, xb;
    srph_transform_to_local_space(&a->transform, &xa, x);
    srph_transform_to_local_space(&b->transform, &xb, x);

    double phi_a = srph_sdf_phi(a->sdf, &xa);
    double phi_b = srph_sdf_phi(b->sdf, &xb);
    double phi = phi_a + phi_b;
   
    if (phi <= 0){
        return 0;
    }
 
    vec3 n = srph_sdf_normal(a->sdf, &xa);

    srph::vec3_t n1(n.x, n.y, n.z);
    n1 = a->get_rotation() * n1;

    n = { n1[0], n1[1], n1[2] };

    //TODO
    srph::vec3_t x1(x->x, x->y, x->z);
    srph::vec3_t va = a->get_velocity(x1);
    srph::vec3_t vb = b->get_velocity(x1);
    srph::vec3_t vr1 = va - vb;

    vec3 vr = { vr1[0], vr1[2], vr1[2] };

    double vrn = srph_vec3_dot(&vr, &n);

    if (vrn <= 0){
        return DBL_MAX;
    }

    // estimate of time to collision
    return phi / vrn;        
}

using namespace srph;

srph::collision_t::collision_t(srph_matter * a, srph_matter * b){
    this->a = a;
    this->b = b;
    intersecting = false;
    t = constant::sigma;

    vec3 c_a, c_b;
    double r_a, r_b;
    srph_matter_sphere_bound(a, constant::sigma, &c_a, &r_a);
    srph_matter_sphere_bound(b, constant::sigma, &c_b, &r_b);

    vec3 d3;
    srph_vec3_subtract(&d3, &c_a, &c_b);
    double d = srph_vec3_length(&d3) - r_a - r_b;
    if (d <= 0){
        srph_bound3 bound_a = a->get_moving_bound(constant::sigma);
        srph_bound3 bound_b = b->get_moving_bound(constant::sigma);

        srph_bound3 bound_i;
        srph_bound3_intersection(&bound_a, &bound_b, &bound_i);

        vec3 xs1[4];
        srph_bound3_vertex(&bound_i, 0, xs1[0].raw);
        srph_bound3_vertex(&bound_i, 3, xs1[1].raw);
        srph_bound3_vertex(&bound_i, 5, xs1[2].raw);
        srph_bound3_vertex(&bound_i, 6, xs1[3].raw);

        srph_opt_sample s;
        srph_opt_nelder_mead(&s, intersection_func, this, xs1, NULL);
        depth = fabs(s.fx);
        x = vec3_t(s.x.x, s.x.y, s.x.z);  

        double iota = constant::iota;
        srph_opt_nelder_mead(&s, time_to_collision_func, this, xs1, &iota);
        t = s.fx;
        intersecting = t <= constant::iota;
        
        // find relative velocity at point 
        vr = a->get_velocity(x) - b->get_velocity(x);
    }
}

bool srph::collision_t::is_intersecting() const {
    return intersecting;
}

double srph::collision_t::get_estimated_time() const {
    return t;
}

void srph::collision_t::resting_correct(){

}

void srph::collision_t::colliding_correct(){
    vec3 x_a1 = { x_a[0], x_a[1], x_a[2] };
    vec3 x_b1 = { x_b[0], x_b[1], x_b[2] };

    // calculate collision impulse magnitude
    auto mata = a->get_material(&x_a1);
    auto matb = b->get_material(&x_b1);

    double CoR = std::max(mata.restitution, matb.restitution);

    double jr = (1.0 + CoR) * vec::dot(vr, n) / (
        1.0 / a->get_mass() + a->get_inverse_angular_mass(x, n) +
        1.0 / b->get_mass() + b->get_inverse_angular_mass(x, n)
    );

    a->apply_impulse_at(-jr * n, x);
    b->apply_impulse_at( jr * n, x);

    // apply friction force
    vec3_t t = vr - vec::dot(vr, n) * n;
    if (t != vec3_t()){
        // no surface friction because impact vector is perpendicular to surface
        t = vec::normalise(t);
        
        double vrt = vec::dot(vr, t); 
        auto mvta = a->get_mass() * vrt;
        auto mvtb = b->get_mass() * vrt;

        double js = std::max(mata.static_friction,  matb.static_friction ) * jr;
        double jd = std::max(mata.dynamic_friction, matb.dynamic_friction) * jr;

        double ka = -(mvta <= js) ? mvta : jd;
        double kb =  (mvtb <= js) ? mvtb : jd;

        a->apply_impulse_at(ka * t, x);
        b->apply_impulse_at(kb * t, x);
    }
}

void srph::collision_t::correct(){
    x_a = a->to_local_space(x);
    x_b = b->to_local_space(x);

    vec3 x1a = { x_a[0], x_a[1], x_a[2] };
    vec3 x1b = { x_b[0], x_b[1], x_b[2] };

    // choose best normal based on smallest second derivative
    auto ja = srph_sdf_jacobian(a->sdf, &x1a);
    auto jb = srph_sdf_jacobian(b->sdf, &x1b);

    vec3 n1;
    if (vec::length(ja) <= vec::length(jb)){
        n1 = srph_sdf_normal(a->sdf, &x1a);
        n = a->get_rotation() * vec3_t(n1.x, n1.y, n1.z);

    } else {
        n1 = srph_sdf_normal(b->sdf, &x1b);
        n = b->get_rotation() * vec3_t(n1.x, n1.y, n1.z);
    }

    // extricate matters 
    double sm = a->get_mass() + b->get_mass();
    a->translate(-depth * n * b->get_mass() / sm);
    b->translate( depth * n * a->get_mass() / sm);
    
    auto vrn = vec::dot(vr, n);

    if (vrn > constant::epsilon){
        colliding_correct();
    } else {
        resting_correct();
    }
}

bool srph::collision_t::comparator_t::operator()(const collision_t & a, const collision_t & b){
    return a.t < b.t;
}
