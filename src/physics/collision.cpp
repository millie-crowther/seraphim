#include "physics/collision.h"

#include <assert.h>
#include <float.h>

#include "maths/matrix.h"
#include "maths/optimise.h"
#include "maths/vector.h"

static double intersection_func(void * data, const vec3 * x){
    srph_matter * a = ((srph_matter **) data)[0];
    srph_matter * b = ((srph_matter **) data)[1];

    vec3 xa, xb;
    srph_matter_to_local_position(a, &xa, x);
    srph_matter_to_local_position(b, &xb, x);

    double phi_a = srph_sdf_phi(a->sdf, &xa);
    double phi_b = srph_sdf_phi(b->sdf, &xb);

    return std::max(phi_a, phi_b);
}

//static double time_to_collision_func(void * data, const vec3 * x){
//    srph_collision * collision = (srph_collision *) data;
//    srph_matter * a = collision->a;
//    srph_matter * b = collision->b;
//
//    vec3 xa, xb;
//    srph_transform_to_local_position(&a->transform, &xa, x);
//    srph_transform_to_local_position(&b->transform, &xb, x);
//
//    double phi_a = srph_sdf_phi(a->sdf, &xa);
//    double phi_b = srph_sdf_phi(b->sdf, &xb);
//    double phi = phi_a + phi_b;
//
//    if (phi <= 0){
//        return 0;
//    }
//
//    vec3 n = srph_sdf_normal(a->sdf, &xa);
//
//    srph::vec3_t n1(n.x, n.y, n.z);
//    n1 = a->get_rotation() * n1;
//
//    n = { n1[0], n1[1], n1[2] };
//
//    //TODO
//    srph::vec3_t x1(x->x, x->y, x->z);
//    srph::vec3_t va = a->get_velocity(x1);
//    srph::vec3_t vb = b->get_velocity(x1);
//    srph::vec3_t vr1 = va - vb;
//
//    vec3 vr = { vr1[0], vr1[2], vr1[2] };
//
//    double vrn = srph_vec3_dot(&vr, &n);
//
//    if (vrn <= 0){
//        return DBL_MAX;
//    }
//
//    // estimate of time to collision
//    return phi / vrn;
//}

//srph_collision::srph_collision(srph_matter * a, srph_matter * b){
//    this->a = a;
//    this->b = b;
//    is_intersecting = false;
//    t = constant::sigma;
//
//    srph_sphere sa, sb;
//    srph_matter_sphere_bound(a, constant::sigma, &sa);
//    srph_matter_sphere_bound(b, constant::sigma, &sb);
//
//    if (srph_sphere_intersect(&sa, &sb)){
//        srph_bound3 bound_a = a->get_moving_bound(constant::sigma);
//        srph_bound3 bound_b = b->get_moving_bound(constant::sigma);
//
//        srph_bound3 bound_i;
//        srph_bound3_intersection(&bound_a, &bound_b, &bound_i);
//
//        vec3 xs1[4];
//        srph_bound3_vertex(&bound_i, 0, xs1[0].raw);
//        srph_bound3_vertex(&bound_i, 3, xs1[1].raw);
//        srph_bound3_vertex(&bound_i, 5, xs1[2].raw);
//        srph_bound3_vertex(&bound_i, 6, xs1[3].raw);
//
//        srph_opt_sample s;
//        srph_opt_nelder_mead(&s, intersection_func, this, xs1, NULL);
//        depth = fabs(s.fx);
//        x = s.x;
//
//        double iota = constant::iota;
//        srph_opt_nelder_mead(&s, time_to_collision_func, this, xs1, &iota);
//        t = s.fx;
//        is_intersecting = t <= constant::iota;
//    }
//}

//void srph_collision::colliding_correct(){
//    // calculate collision impulse magnitude
//    auto mata = a->get_material(&xa);
//    auto matb = b->get_material(&xb);
//
//    double CoR = std::max(mata.restitution, matb.restitution);
//
//    srph::vec3_t x1(x.x, x.y, x.z);
//
//    double jr = (1.0 + CoR) * vec::dot(vr, n) / (
//        1.0 / srph_matter_mass(a) + a->get_inverse_angular_mass(x1, n) +
//        1.0 / srph_matter_mass(b) + b->get_inverse_angular_mass(x1, n)
//    );
//
//    a->apply_impulse_at(n * -jr, x1);
//    b->apply_impulse_at(n *  jr, x1);
//
//    // apply friction force
//    vec3_t t = vr - n * vec::dot(vr, n);
//    if (t != vec3_t()){
//        // no surface friction because impact vector is perpendicular to surface
//        vec3 t1 = { t[0], t[1], t[2] };
//        srph_vec3_normalise(&t1, &t1);
//        t = vec3_t(t1.x, t1.y, t1.z);
//
//        double vrt = vec::dot(vr, t);
//        auto mvta = srph_matter_mass(a) * vrt;
//        auto mvtb = srph_matter_mass(b) * vrt;
//
//        double js = std::max(mata.static_friction,  matb.static_friction ) * jr;
//        double jd = std::max(mata.dynamic_friction, matb.dynamic_friction) * jr;
//
//        double ka = -(mvta <= js) ? mvta : jd;
//        double kb =  (mvtb <= js) ? mvtb : jd;
//
//        a->apply_impulse_at(t * ka, x1);
//        b->apply_impulse_at(t * kb, x1);
//    }
//}

//void srph_collision::correct(){
//    srph_transform_to_local_position(&a->transform, &xa, &x);
//    srph_transform_to_local_position(&b->transform, &xb, &x);
//
//    // find relative velocity at point
//    srph::vec3_t x1(x.x, x.y, x.z);
//    vr = a->get_velocity(x1) - b->get_velocity(x1);
//    colliding_correct();
//}


static void contact_correct(srph_matter * a, srph_matter * b, srph_deform * bx){
    assert(a != NULL && b != NULL && bx != NULL);

    // check that it is actually colliding at this point
    vec3 x, xa;
    srph_matter_to_global_position(b, &x, &bx->x0);
    srph_matter_to_local_position(a, &xa, &x);

    double phi = srph_sdf_phi(a->sdf, &xa);
    if (phi > srph::constant::epsilon){
        return;
    }

    // check that relative velocity at this point is incoming
    vec3 va, vb, vr;
    srph_matter_velocity(a, &x, &va);
    srph_matter_velocity(b, &x, &vb);
    srph_vec3_subtract(&vr, &va, &vb);

    vec3 n = srph_sdf_normal(a->sdf, &xa);
    srph_matter_to_global_direction(a, NULL, &n, &n);
    double vrn = srph_vec3_dot(&vr, &n);
    if (vrn < -srph::constant::epsilon){
        return;
    }

    // calculate collision impulse magnitude
    srph_material mata, matb;
    srph_matter_material(a, &mata);
    srph_matter_material(b, &matb);

    double CoR = fmax(mata.restitution, matb.restitution);

    double jr = (1.0 + CoR) * vrn / (
        1.0 / srph_matter_mass(a) + srph_matter_inverse_angular_mass(a, &x, &n) +
        1.0 / srph_matter_mass(b) + srph_matter_inverse_angular_mass(b, &x, &n)
    );

    vec3 ja, jb;
    srph_vec3_scale(&ja, &n, -jr);
    srph_vec3_scale(&jb, &n,  jr);

    srph_matter_apply_impulse_at(a, &x, &ja);
    srph_matter_apply_impulse_at(b, &x, &jb);

    // apply friction force
    vec3 t = n;
    srph_vec3_scale(&n, &n, -vrn);
    srph_vec3_add(&t, &t, &vr);
    if (srph_vec3_length(&t) < srph::constant::epsilon){
        return;
    }

    srph_vec3_normalise(&t, &t);

    double vrt  = srph_vec3_dot(&vr, &t);
    double mvta = srph_matter_mass(a) * vrt;
    double mvtb = srph_matter_mass(b) * vrt;

    double js = fmax(mata.static_friction,  matb.static_friction ) * jr;
    double jd = fmax(mata.dynamic_friction, matb.dynamic_friction) * jr;

    double ka = -(mvta <= js) ? mvta : jd;
    double kb =  (mvtb <= js) ? mvtb : jd;

    vec3 ta, tb;
    srph_vec3_scale(&ta, &t, ka);
    srph_vec3_scale(&tb, &t, kb);

    srph_matter_apply_impulse_at(a, &x, &ta);
    srph_matter_apply_impulse_at(b, &x, &tb);
}

void srph_collision_correct(srph_collision *self) {
    assert(self != NULL);

    for (int i = 0; i < 2; i++){
        srph_matter * a = self->ms[i];
        srph_matter * b = self->ms[1 - i];

        for (size_t j = 0; j < b->deformations.size; j++){
            contact_correct(a, b, b->deformations.data[j]);
        }
    }
}

bool srph_collision_is_detected(srph_collision * c, srph_substance * a, srph_substance * b, double dt){
    if (srph_matter_is_at_rest(&a->matter) && srph_matter_is_at_rest(&b->matter)){
        return false;
    }

    srph_sphere sa, sb;
    srph_matter_sphere_bound(&a->matter, dt, &sa);
    srph_matter_sphere_bound(&b->matter, dt, &sb);

    if (!srph_sphere_intersect(&sa, &sb)){
        return false;
    }

    vec3 r;
    srph_vec3_fill(&r, fmin(sa.r, sb.r) / 2);

    vec3 xa, xb;
    srph_vec3_scale(&xa, &sa.c, sb.r / (sa.r + sb.r));
    srph_vec3_scale(&xb, &sb.c, sa.r / (sa.r + sb.r));

    vec3 x;
    srph_vec3_add(&x, &xa, &xb);
    srph_vec3_subtract(&x, &x, &r);

    vec3 xs[4] = { x, x, x, x };
    xs[1].x += r.x;
    xs[2].y += r.y;
    xs[3].z += r.z;

    srph_matter * matters[] = { &a->matter, &b->matter };
    srph_opt_sample s;
    double threshold = 0.0;
    srph_opt_nelder_mead(&s, intersection_func, matters, xs, &threshold);

    bool is_colliding = s.fx <= 0;
    c->ms[0] = &a->matter;
    c->ms[1] = &b->matter;
    c->x = s.x;
    
    if (is_colliding){
        srph_matter_add_deformation(&a->matter, &s.x, srph_deform_type_collision);
        srph_matter_add_deformation(&b->matter, &s.x, srph_deform_type_collision);
    }
    
    return is_colliding;
}

void srph_collision_resolve_interpenetration_constraint(srph_collision * c) {
    assert(c->ms[0]->is_rigid && c->ms[1]->is_rigid);

    for (int i = 0; i < 2; i++){
        srph_matter * a = c->ms[i];
        srph_matter * b = c->ms[1 - i];

        double ratio = srph_matter_mass(a) / (srph_matter_mass(a) + srph_matter_mass(b));

        for (size_t j = 0; j < b->deformations.size; j++){
            srph_deform * d = b->deformations.data[j];
            vec3 xa, xb;
            srph_matter_to_global_position(b, &xb, &d->x0);
            srph_matter_to_local_position(a, &xa, &xb);

            double phi = srph_sdf_phi(a->sdf, &xa) + srph_sdf_phi(b->sdf, &d->x0);
            if (phi <= 0){
                vec3 n = srph_sdf_normal(a->sdf, &xa);
                srph_matter_to_global_direction(a, &d->x, &n, &n);
                srph_vec3_scale(&n, &n, -phi * ratio);
                srph_transform_translate(&b->transform, &n);
            }
        }
    }
}