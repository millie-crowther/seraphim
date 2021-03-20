#include "physics/collision.h"

#include <float.h>

#include "maths/matrix.h"
#include "maths/optimise.h"
#include "maths/vector.h"

static double intersection_func(void * data, const vec3 * x){
    srph_collision * collision = (srph_collision *) data;
    srph_matter * a = collision->a;
    srph_matter * b = collision->b;
    
    vec3 xa, xb;
    srph_transform_to_local_space(&a->transform, &xa, x);
    srph_transform_to_local_space(&b->transform, &xb, x);

    double phi_a = srph_sdf_phi(a->sdf, &xa);
    double phi_b = srph_sdf_phi(b->sdf, &xb);

    return std::max(phi_a, phi_b);
}

static double intersection_func1(void * data, const vec3 * x){
    srph_matter * a = ((srph_matter **) data)[0];
    srph_matter * b = ((srph_matter **) data)[1];
    
    vec3 xa, xb;
    srph_transform_to_local_space(&a->transform, &xa, x);
    srph_transform_to_local_space(&b->transform, &xb, x);

    double phi_a = srph_sdf_phi(a->sdf, &xa);
    double phi_b = srph_sdf_phi(b->sdf, &xb);

    return std::max(phi_a, phi_b);
}

static double time_to_collision_func(void * data, const vec3 * x){
    srph_collision * collision = (srph_collision *) data;
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

srph_collision::srph_collision(srph_matter * a, srph_matter * b){
    this->a = a;
    this->b = b;
    is_intersecting = false;
    t = constant::sigma;

    srph_sphere sa, sb;
    srph_matter_sphere_bound(a, constant::sigma, &sa);
    srph_matter_sphere_bound(b, constant::sigma, &sb);

    if (srph_sphere_intersect(&sa, &sb)){
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
        x = s.x; 

        double iota = constant::iota;
        srph_opt_nelder_mead(&s, time_to_collision_func, this, xs1, &iota);
        t = s.fx;
        is_intersecting = t <= constant::iota;
    }
}

void srph_collision::colliding_correct(){
    // calculate collision impulse magnitude
    auto mata = a->get_material(&xa);
    auto matb = b->get_material(&xb);

    double CoR = std::max(mata.restitution, matb.restitution);

    srph::vec3_t x1(x.x, x.y, x.z);

    double jr = (1.0 + CoR) * vec::dot(vr, n) / (
        1.0 / srph_matter_mass(a) + a->get_inverse_angular_mass(x1, n) +
        1.0 / srph_matter_mass(b) + b->get_inverse_angular_mass(x1, n)
    );

    a->apply_impulse_at(n * -jr, x1);
    b->apply_impulse_at(n *  jr, x1);

    // apply friction force
    vec3_t t = vr - n * vec::dot(vr, n);
    if (t != vec3_t()){
        // no surface friction because impact vector is perpendicular to surface
        vec3 t1 = { t[0], t[1], t[2] };
        srph_vec3_normalise(&t1, &t1);
        t = vec3_t(t1.x, t1.y, t1.z);
        
        double vrt = vec::dot(vr, t); 
        auto mvta = srph_matter_mass(a) * vrt;
        auto mvtb = srph_matter_mass(b) * vrt;

        double js = std::max(mata.static_friction,  matb.static_friction ) * jr;
        double jd = std::max(mata.dynamic_friction, matb.dynamic_friction) * jr;

        double ka = -(mvta <= js) ? mvta : jd;
        double kb =  (mvtb <= js) ? mvtb : jd;

        a->apply_impulse_at(t * ka, x1);
        b->apply_impulse_at(t * kb, x1);
    }
}

void srph_collision::correct(){
    srph_transform_to_local_space(&a->transform, &xa, &x);
    srph_transform_to_local_space(&b->transform, &xb, &x);
 
    // choose best normal based on smallest second derivative
    auto ja = srph_sdf_jacobian(a->sdf, &xa);
    auto jb = srph_sdf_jacobian(b->sdf, &xb);

    vec3 n1;
    if (vec::length(ja) <= vec::length(jb)){
        n1 = srph_sdf_normal(a->sdf, &xa);
        n = a->get_rotation() * vec3_t(n1.x, n1.y, n1.z);

    } else {
        n1 = srph_sdf_normal(b->sdf, &xb);
        n = b->get_rotation() * vec3_t(n1.x, n1.y, n1.z);
    }

    // extricate matters 
    double sm = srph_matter_mass(a) + srph_matter_mass(b);
    a->translate(n * -depth * srph_matter_mass(b) / sm);
    b->translate(n *  depth * srph_matter_mass(a) / sm);
    
    // find relative velocity at point 
    srph::vec3_t x1(x.x, x.y, x.z);
    vr = a->get_velocity(x1) - b->get_velocity(x1);
    colliding_correct();
}

bool srph_collision::comparator_t::operator()(const srph_collision & a, const srph_collision & b){
    return a.t < b.t;
}

bool srph_collision_is_detected(srph_substance * a, srph_substance * b, double dt){
    srph_sphere sa, sb;
    srph_matter_sphere_bound(&a->matter, dt, &sa);
    srph_matter_sphere_bound(&b->matter, dt, &sb);

    if (!srph_sphere_intersect(&sa, &sb)){
        return false;
    }

    srph_bound3 bound_a = a->matter.get_moving_bound(dt);
    srph_bound3 bound_b = b->matter.get_moving_bound(dt);

    srph_bound3 bound_i;
    srph_bound3_intersection(&bound_a, &bound_b, &bound_i);

    vec3 xs1[4];
    srph_bound3_vertex(&bound_i, 0, xs1[0].raw);
    srph_bound3_vertex(&bound_i, 3, xs1[1].raw);
    srph_bound3_vertex(&bound_i, 5, xs1[2].raw);
    srph_bound3_vertex(&bound_i, 6, xs1[3].raw);

    srph_matter * matters[] = { &a->matter, &b->matter };
    srph_opt_sample s;
    srph_opt_nelder_mead(&s, intersection_func1, matters, xs1, NULL);
    return s.fx < 0;
}

void srph_collision_push_constraints(srph_constraint_array * cs, srph_substance * a, srph_substance * b){
    // TODO
}
