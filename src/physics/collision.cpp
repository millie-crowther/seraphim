#include "physics/collision.h"

#include "maths/matrix.h"
#include "maths/vector.h"

#include "maths/optimise.h"

#include <map>
#include <queue>

using namespace srph;

srph::collision_t::collision_t(srph_matter * a, srph_matter * b){
    this->a = a;
    this->b = b;
    intersecting = false;
    t = constant::sigma;

    auto f = [a, b](const vec3_t & x) -> double {
        vec3_t xa = a->transform.to_local_space(x);
        vec3_t xb = b->transform.to_local_space(x);

        vec3 x1a, x1b;
        srph_vec3_set(&x1a, xa[0], xa[1], xa[2]);
        srph_vec3_set(&x1b, xb[0], xb[1], xb[2]);

        double phi_a = srph_sdf_phi(a->sdf, &x1a);
        double phi_b = srph_sdf_phi(b->sdf, &x1b);

        return std::max(phi_a, phi_b);
    };

    srph_bound3 bound_a = a->get_moving_bound(constant::sigma);
    srph_bound3 bound_b = b->get_moving_bound(constant::sigma);

    srph_bound3 bound_i;
    srph_bound3_intersection(&bound_a, &bound_b, &bound_i);

    vec3 xs1[4];
    srph_bound3_vertex(&bound_i, 0, xs1[0].raw);
    srph_bound3_vertex(&bound_i, 3, xs1[1].raw);
    srph_bound3_vertex(&bound_i, 5, xs1[2].raw);
    srph_bound3_vertex(&bound_i, 6, xs1[3].raw);

    std::array<vec3_t, 4> xs = {
        vec3_t(xs1[0].x, xs1[0].y, xs1[0].z),
        vec3_t(xs1[1].x, xs1[1].y, xs1[1].z),
        vec3_t(xs1[2].x, xs1[2].y, xs1[2].z),
        vec3_t(xs1[3].x, xs1[3].y, xs1[3].z)
    };

    if (srph_bound3_is_valid(&bound_i)){
        auto result = srph::optimise::nelder_mead(f, xs);
        depth = std::abs(result.fx);
        x = result.x;  

        auto ttc = [this](const vec3_t & x){ return time_to_collision(x); };
        auto t_result = srph::optimise::nelder_mead(ttc, xs, constant::iota);
        t = t_result.fx;
        intersecting = t < constant::iota;
    }
    
    // find relative velocity at point 
    vr = a->get_velocity(x) - b->get_velocity(x);
}

bool srph::collision_t::is_intersecting() const {
    return intersecting;
}

double srph::collision_t::get_estimated_time() const {
    return t;
}

double collision_t::time_to_collision(const vec3_t & x){
    vec3_t xa = a->transform.to_local_space(x);
    vec3_t xb = b->transform.to_local_space(x);

    vec3 x1a, x1b;
    srph_vec3_set(&x1a, xa[0], xa[1], xa[2]);
    srph_vec3_set(&x1b, xb[0], xb[1], xb[2]);

    double phi_a = srph_sdf_phi(a->sdf, &x1a);
    double phi_b = srph_sdf_phi(b->sdf, &x1b);
    double phi = phi_a + phi_b;
   
    if (phi <= 0){
        return 0;
    }
 
    vec3 n = srph_sdf_normal(a->sdf, &x1a);

    vec3_t n1(n.x, n.y, n.z);
    n1 = a->get_rotation() * n1;

    srph_vec3_set(&n, n1[0], n1[1], n1[2]);

    //TODO
    vec3_t va = a->get_velocity(x);
    vec3_t vb = b->get_velocity(x);
    vec3_t vr1 = va - vb;

    vec3 vr;
    srph_vec3_set(&vr, vr1[0], vr1[2], vr1[2]);

    double vrn = srph_vec3_dot(&vr, &n);

    if (vrn <= 0){
        return std::numeric_limits<double>::max();
    }

    // estimate of time to collision
    return phi / vrn;        
}

void srph::collision_t::resting_correct(){

}

void srph::collision_t::colliding_correct(){
    // calculate collision impulse magnitude
    auto mata = a->get_material(x_a);
    auto matb = b->get_material(x_b);

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

    vec3 x1a, x1b;
    srph_vec3_set(&x1a, x_a[0], x_a[1], x_a[2]);
    srph_vec3_set(&x1b, x_b[0], x_b[1], x_b[2]);

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
