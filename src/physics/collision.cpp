//
// Created by millie on 14/04/2021.
//
#include "physics/collision.h"

#include <assert.h>

#include "maths/optimise.h"
#include "core/constant.h"

static int axis_comparator(const substance_t *a, const substance_t *b, int axis) {
    const sphere_t *sa = &a->matter.bounding_sphere;
    const sphere_t *sb = &b->matter.bounding_sphere;

    double lower_a = sa->c.v[axis] - sa->r;
    double lower_b = sb->c.v[axis] - sb->r;

    if (lower_a < lower_b) {
        return -1;
    } else if (lower_a > lower_b) {
        return 1;
    } else {
        return 0;
    }
}

static int x_comparator(const void *a, const void *b) {
    return axis_comparator(*(const substance_t **) a, *(const substance_t **) b, X_AXIS);
}

static void collision_broad_phase(
        substance_t *substance_pointers,
        size_t num_substances,
        srph_collision_array *cs
) {
    srph_array_clear(cs);

    srph_array(substance_t *) substances{};
    srph_array_init(&substances);

    for (size_t i = 0; i < num_substances; i++) {
        srph_array_push_back(&substances);
        substances.data[i] = &substance_pointers[i];
    }

    srph_array_sort(&substances, x_comparator);

    for (size_t i = 0; i < num_substances; i++) {
        matter_t *a = &substances.data[i]->matter;
        sphere_t *sa = &a->bounding_sphere;

        for (size_t j = i + 1; j < num_substances; j++) {
            matter_t *b = &substances.data[j]->matter;
            sphere_t *sb = &b->bounding_sphere;

            if (sa->c.x + sa->r < sb->c.x - sb->r) {
                break;
            }

            if ((a->is_at_rest || a->is_static) && (b->is_at_rest || b->is_static)) {
                continue;
            }

            srph_array_push_back(cs);
            cs->last->ms[0] = a;
            cs->last->ms[1] = b;
        }
    }
}


static double intersection_func(void *data, const vec3 *x) {
    matter_t *a = ((matter_t **) data)[0];
    matter_t *b = ((matter_t **) data)[1];

    vec3 xa, xb;
    srph_matter_to_local_position(a, &xa, x);
    srph_matter_to_local_position(b, &xb, x);

    double phi_a = sdf_distance(a->sdf, &xa);
    double phi_b = sdf_distance(b->sdf, &xb);

    return std::max(phi_a, phi_b);
}

//static double time_to_collision_func(void * data, const vec3 * x){
//    collision_t * collision = (collision_t *) data;
//    matter_t * a = collision->a;
//    matter_t * b = collision->b;
//
//    vec3 xa, xb;
//    srph_transform_to_local_position(&a->transform, &xa, x);
//    srph_transform_to_local_position(&b->transform, &xb, x);
//
//    double phi_a = sdf_distance(a->sdf, &xa);
//    double phi_b = sdf_distance(b->sdf, &xb);
//    double distance_function = phi_a + phi_b;
//
//    if (distance_function <= 0){
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
//    return distance_function / vrn;
//}

static void contact_correct(matter_t *a, matter_t *b, srph_deform *xb, double dt) {
    // check that deformation is a collision deformation
    if (xb->type != srph_deform_type_collision) {
        return;
    }

    // check that relative velocity at this point is incoming
    vec3 x, xa;
    srph_matter_to_global_position(b, &x, &xb->x0);

    srph_matter_to_local_position(a, &xa, &x);

    vec3 va, vb, vr;
    srph_matter_velocity(a, &x, &va);
    srph_matter_velocity(b, &x, &vb);
    vec3_subtract(&vr, &vb, &va);

    vec3 n;
    if (srph_sdf_discontinuity(a->sdf, &xa) < srph_sdf_discontinuity(b->sdf, &xb->x0)) {
        n = srph_sdf_normal(a->sdf, &xa);
        srph_matter_to_global_direction(a, NULL, &n, &n);
    } else {
        n = srph_sdf_normal(b->sdf, &xb->x0);
        vec3_negative(&n, &n);
        srph_matter_to_global_direction(b, NULL, &n, &n);
    }

    double vrn = vec3_dot(&vr, &n);
    if (vrn >= 0) {
        return;
    }

    // check that it is actually colliding at this point
    double phi = sdf_distance(a->sdf, &xa);
    if (phi > fabs(vrn) * dt) {
        return;
    }

    // calculate collision impulse magnitude
    material_t mata, matb;
    srph_matter_material(a, &mata, nullptr);
    srph_matter_material(b, &matb, nullptr);

    double CoR = fmax(mata.restitution, matb.restitution);

    double inverse_mass =
            srph_matter_inverse_mass(a) +
            srph_matter_inverse_mass(b) +
            srph_matter_inverse_angular_mass(a, &x, &n) +
            srph_matter_inverse_angular_mass(b, &x, &n);

    if (inverse_mass == 0.0) {
        return;
    }

    double jr = -(1.0 + CoR) * vrn / inverse_mass;
    vec3 j;
    vec3_multiply_f(&j, &n, -jr);
    matter_apply_impulse(a, b, &x, &j);

    // apply friction force
    vec3 t;
    vec3_multiply_f(&t, &n, -vrn);
    vec3_add(&t, &t, &vr);
    if (vec3_length(&t) < epsilon) {
        return;
    }

    vec3_normalize(&t, &t);

    double mvrt = (srph_matter_mass(a) + srph_matter_mass(b)) * vec3_dot(&vr, &t);

    double js = fmax(mata.static_friction, matb.static_friction) * jr;
    double jd = fmax(mata.dynamic_friction, matb.dynamic_friction) * jr;

    double ka = -(mvrt <= js) ? mvrt : jd;

    vec3 fr;
    vec3_multiply_f(&fr, &t, ka);
    matter_apply_impulse(a, b, &x, &fr);
}

static void collision_resolve_velocity_constraint(collision_t *self, double dt) {
    for (int i = 0; i < 2; i++) {
        matter_t *a = self->ms[i];
        matter_t *b = self->ms[1 - i];

        for (size_t j = 0; j < b->deformations.size; j++) {
            contact_correct(a, b, b->deformations.data[j], dt);
        }
    }
}

static void collision_generate_manifold(collision_t *c, double dt) {
    srph_array_init(&c->manifold);

    sphere_t *sa = &c->ms[0]->bounding_sphere;
    sphere_t *sb = &c->ms[1]->bounding_sphere;

    double r_elem = fmin(sa->r, sb->r) / 2;
    vec3 r = {{r_elem, r_elem, r_elem}};
    vec3 xa, xb;
    double radius_sum = sa->r + sb->r;

    if (radius_sum <= 0) {
        return;
    }

    vec3_multiply_f(&xa, &sa->c, sb->r / radius_sum);
    vec3_multiply_f(&xb, &sa->c, sa->r / radius_sum);

    vec3 x;
    vec3_add(&x, &xa, &xb);
    vec3_subtract(&x, &x, &r);

    vec3 xs[4] = {x, x, x, x};
    xs[1].x += r.x;
    xs[2].y += r.y;
    xs[3].z += r.z;

    srph_opt_sample s;
    double threshold = 0.0;
    srph_opt_nelder_mead(&s, intersection_func, c->ms, xs, &threshold);

    if (s.fx <= 0) {
        srph_matter_add_deformation(c->ms[0], &s.x, srph_deform_type_collision);
        srph_matter_add_deformation(c->ms[1], &s.x, srph_deform_type_collision);

        srph_array_push_back(&c->manifold);
        c->manifold.data[0] = s.x;
    }
}

static void collision_resolve_interpenetration_constraint(collision_t *c) {
    assert(c->ms[0]->is_rigid && c->ms[1]->is_rigid);

    for (int i = 0; i < 2; i++) {
        matter_t *a = c->ms[i];
        matter_t *b = c->ms[1 - i];

        double ratio = srph_matter_mass(a) / (srph_matter_mass(a) + srph_matter_mass(b));

        for (size_t j = 0; j < b->deformations.size; j++) {
            srph_deform *d = b->deformations.data[j];
            vec3 xa, x;
            srph_matter_to_global_position(b, &x, &d->x0);
            srph_matter_to_local_position(a, &xa, &x);

            double phi = sdf_distance(a->sdf, &xa) + sdf_distance(b->sdf, &d->x0);
            if (phi <= 0) {
                vec3 n = srph_sdf_normal(a->sdf, &xa);
                srph_matter_to_global_direction(a, NULL, &n, &n);
                vec3_multiply_f(&n, &n, -phi * ratio);
                srph_transform_translate(&b->transform, &n);
            }
        }
    }
}

static bool is_colliding_in_bound(matter_t **ms, bound3_t *bound) {
    vec3 radius;
    srph_bound3_radius(bound, &radius);
    if (vec3_length(&radius) <= epsilon) {
        return false;
    }

    bound3_t sub_bounds[2];
    srph_bound3_bisect(bound, sub_bounds);
    double sub_bound_distances[2];

    for (int sub_bound_index = 0; sub_bound_index < 2; sub_bound_index++) {
        vec3 global_position;
        double phis[2];
        srph_bound3_midpoint(&sub_bounds[sub_bound_index], &global_position);

        for (int matter_index = 0; matter_index < 2; matter_index++) {
            vec3 local_position;
            srph_matter_to_local_position(ms[matter_index], &local_position, &global_position);
            phis[matter_index] = sdf_distance(ms[matter_index]->sdf, &local_position);
        }

        sub_bound_distances[sub_bound_index] = phis[0] + phis[1];

        if (sub_bound_distances[sub_bound_index] <= epsilon) {
            return true;
        }

        srph_bound3_radius(&sub_bounds[sub_bound_index], &radius);
        double radius_length = vec3_length(&radius);

        if (phis[0] >= radius_length || phis[1] >= radius_length) {
            return false;
        }
    }

    if (sub_bound_distances[0] < sub_bound_distances[1]) {
        return is_colliding_in_bound(ms, &sub_bounds[0]) || is_colliding_in_bound(ms, &sub_bounds[1]);
    } else {
        return is_colliding_in_bound(ms, &sub_bounds[1]) || is_colliding_in_bound(ms, &sub_bounds[0]);
    }
}

static bool collision_narrow_phase(collision_t *c) {
    bound3_t bounds[2];
    for (int matter_index = 0; matter_index < 2; matter_index++) {
        sphere_t *bounding_sphere = &c->ms[matter_index]->bounding_sphere;
        vec3_subtract_f(&bounds[matter_index].lower, &bounding_sphere->c, bounding_sphere->r);
        vec3_add_f(&bounds[matter_index].upper, &bounding_sphere->c, bounding_sphere->r);
    }

    srph_bound3_intersection(&bounds[0], &bounds[1], &c->bound);

    if (!bound3_is_valid(&c->bound)) {
        return false;
    }

    if (is_colliding_in_bound(c->ms, &c->bound)){
        c->ms[0]->has_collided = true;
        c->ms[1]->has_collided = true;
        return true;
    }

    return false;
}

void collision_detect(substance_t *substance_ptrs, size_t num_substances, srph_collision_array *cs, double dt) {
    // clear collisions from last iteration
    for (size_t i = 0; i < cs->size; i++) {
        srph_array_clear(&cs->data[i].manifold);
    }
    srph_array_clear(cs);

    srph_collision_array broad_phase_collisions;
    srph_array_init(&broad_phase_collisions);

    collision_broad_phase(substance_ptrs, num_substances, &broad_phase_collisions);

    for (size_t i = 0; i < broad_phase_collisions.size; i++) {
        collision_t *collision = &broad_phase_collisions.data[i];
        if (collision_narrow_phase(collision)) {
            srph_array_push_back(cs);
            *(cs->last) = *collision;
            collision_generate_manifold(cs->last, dt);
        }
    }

    srph_array_clear(&broad_phase_collisions);
}

void collision_resolve(collision_t *self, double dt) {
    collision_resolve_velocity_constraint(self, dt);
    collision_resolve_interpenetration_constraint(self);
}
