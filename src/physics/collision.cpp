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

static void find_contact_points(srph_array * xs, srph_matter * a, srph_matter * b){
    for (uint32_t i = 0; i < a->sdf->vertices.size; i++){
        vec3 * x = (vec3 *) srph_array_at(&a->sdf->vertices, i);
        vec3 x_global, x_local_b;
        srph_transform_to_global_space(&a->transform, &x_global, x);
        srph_transform_to_local_space(&b->transform, &x_local_b, &x_global);

        if (srph_sdf_contains(b->sdf, &x_local_b)){
            *((vec3 *) srph_array_push_back(xs)) = x_global;
        }
    }
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

    if (is_intersecting){
        srph_array xs;
        srph_array_create(&xs, sizeof(vec3));

        find_contact_points(&xs, a, b);
        find_contact_points(&xs, b, a);
        
        vec3 cx = srph_vec3_zero;
        for (uint32_t i = 0; i < xs.size; i++){
            srph_vec3_add(&cx, &cx, (vec3 *) srph_array_at(&xs, i));  
        }

        if (!srph_array_is_empty(&xs)){
            srph_vec3_scale(&cx, &cx, 1.0 / (double) xs.size);
            x = cx;
        }

        srph_array_destroy(&xs);
    }
}

void srph_collision::colliding_correct(){
    // calculate collision impulse magnitude
    auto mata = a->get_material(&xa);
    auto matb = b->get_material(&xb);

    double CoR = std::max(mata.restitution, matb.restitution);

    srph::vec3_t x1(x.x, x.y, x.z);

    double jr = (1.0 + CoR) * vec::dot(vr, n) / (
        1.0 / a->get_mass() + a->get_inverse_angular_mass(x1, n) +
        1.0 / b->get_mass() + b->get_inverse_angular_mass(x1, n)
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
        auto mvta = a->get_mass() * vrt;
        auto mvtb = b->get_mass() * vrt;

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

    srph_sdf_add_sample(a->sdf, &xa);
    srph_sdf_add_sample(b->sdf, &xb);
 
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
    double sm = a->get_mass() + b->get_mass();
    a->translate(n * -depth * b->get_mass() / sm);
    b->translate(n *  depth * a->get_mass() / sm);
    
    // find relative velocity at point 
    srph::vec3_t x1(x.x, x.y, x.z);
    vr = a->get_velocity(x1) - b->get_velocity(x1);
    colliding_correct();
}

bool srph_collision::comparator_t::operator()(const srph_collision & a, const srph_collision & b){
    return a.t < b.t;
}
