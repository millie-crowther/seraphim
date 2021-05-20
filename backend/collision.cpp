//
// Created by millie on 14/04/2021.
//
#include "collision.h"

#include <assert.h>

#include "../common/constant.h"
#include "optimise.h"

static int axis_comparator(const substance_t *a, const substance_t *b, int axis) {
    const sphere_t *sa = &a->bounding_sphere;
    const sphere_t *sb = &b->bounding_sphere;

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
    return axis_comparator(*(const substance_t **)a, *(const substance_t **)b,
                           X_AXIS);
}

static void collision_broad_phase(substance_t *substance_pointers,
                                  size_t num_substances, collision_array_t *cs) {
    array_clear(cs);

    array_t(substance_t *) substances{};
    array_create(&substances);

    for (size_t i = 0; i < num_substances; i++) {
        array_push_back(&substances);
        substances.data[i] = &substance_pointers[i];
    }

    array_sort(&substances, x_comparator);

    for (size_t i = 0; i < num_substances; i++) {
        substance_t *a = substances.data[i];
        sphere_t *sa = &a->bounding_sphere;

        for (size_t j = i + 1; j < num_substances; j++) {
            substance_t *b = substances.data[j];
            sphere_t *sb = &b->bounding_sphere;

            if (sa->c.x + sa->r < sb->c.x - sb->r) {
                break;
            }

            if ((a->matter.is_at_rest || a->matter.is_static) &&
                (b->matter.is_at_rest || b->matter.is_static)) {
                continue;
            }

            array_push_back(cs);
            cs->last->substances[0] = a;
            cs->last->substances[1] = b;
        }
    }
}

static double intersection_func(void *data, const vec3 *x) {
    substance_t *a = ((substance_t **)data)[0];
    substance_t *b = ((substance_t **)data)[1];

    vec3 xa, xb;
    matter_to_local_position(&a->matter, &xa, x);
    matter_to_local_position(&b->matter, &xb, x);

    double phi_a = sdf_distance(a->matter.sdf, &xa);
    double phi_b = sdf_distance(b->matter.sdf, &xb);

    return fmax(phi_a, phi_b);
}

static void contact_correct(substance_t *sa, substance_t *sb, deform_t *xb,
                            double dt) {
    matter_t *a = &sa->matter;
    matter_t *b = &sb->matter;

    // check that deformation is a collision deformation
    if (xb->type != deform_type_collision) {
        return;
    }
    // check that relative velocity at this point is incoming
    vec3 x, xa;
    matter_to_global_position(b, &x, &xb->x0);

    matter_to_local_position(a, &xa, &x);

    vec3 va, vb, vr;
    substance_velocity_at(sa, &x, &va);
    substance_velocity_at(sb, &x, &vb);
    vec3_subtract(&vr, &vb, &va);

    vec3 n;
    if (sdf_discontinuity(a->sdf, &xa) <
        sdf_discontinuity(b->sdf, &xb->x0)) {
        n = sdf_normal(a->sdf, &xa);
        matter_to_global_direction(a, NULL, &n, &n);
    } else {
        n = sdf_normal(b->sdf, &xb->x0);
        vec3_negative(&n, &n);
        matter_to_global_direction(b, NULL, &n, &n);
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
    matter_material(a, &mata, NULL);
    matter_material(b, &matb, NULL);

    double CoR = fmax(mata.restitution, matb.restitution);

    double inverse_mass = substance_inverse_mass(sa) + substance_inverse_mass(sb) +
                          substance_inverse_angular_mass(sa, &x, &n) +
                          substance_inverse_angular_mass(sb, &x, &n);

    if (inverse_mass == 0.0) {
        return;
    }

    double jr = -(1.0 + CoR) * vrn / inverse_mass;
    vec3 j;
    vec3_multiply_f(&j, &n, -jr);
    substance_apply_impulse(sa, sb, &x, &j);

    // apply friction force
    vec3 t;
    vec3_multiply_f(&t, &n, -vrn);
    vec3_add(&t, &t, &vr);
    if (vec3_length(&t) < epsilon) {
        return;
    }

    vec3_normalize(&t, &t);

    double mvrt = (substance_mass(sa) + substance_mass(sb)) * vec3_dot(&vr, &t);

    double js = fmax(mata.static_friction, matb.static_friction) * jr;
    double jd = fmax(mata.dynamic_friction, matb.dynamic_friction) * jr;

    double ka = -(mvrt <= js) ? mvrt : jd;

    vec3 fr;
    vec3_multiply_f(&fr, &t, ka);
    substance_apply_impulse(sa, sb, &x, &fr);
}

static void collision_resolve_velocity_constraint(collision_t *self, double dt) {
    for (int i = 0; i < 2; i++) {
        substance_t *a = self->substances[i];
        substance_t *b = self->substances[1 - i];

        for (size_t j = 0; j < b->matter.deformations.size; j++) {
            contact_correct(a, b, b->matter.deformations.data[j], dt);
        }
    }
}

static void collision_generate_manifold(collision_t *c, double dt) {
    array_create(&c->manifold);

    sphere_t *sa = &c->substances[0]->bounding_sphere;
    sphere_t *sb = &c->substances[1]->bounding_sphere;

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

    opt_sample_t s;
    double threshold = 0.0;
    opt_nelder_mead(&s, intersection_func, c->substances, xs, &threshold);

    if (s.fx <= 0) {
        matter_add_deformation(&c->substances[0]->matter, &s.x,
                               deform_type_collision);
        matter_add_deformation(&c->substances[1]->matter, &s.x,
                               deform_type_collision);

        array_push_back(&c->manifold);
        c->manifold.data[0] = s.x;
    }
}

static void collision_resolve_interpenetration_constraint(collision_t *c) {
    for (int i = 0; i < 2; i++) {
        substance_t *sa = c->substances[i];
        substance_t *sb = c->substances[1 - i];

        matter_t *a = &sa->matter;
        matter_t *b = &sb->matter;

        double ratio =
            substance_mass(sa) / (substance_mass(sa) + substance_mass(sb));

        for (size_t j = 0; j < b->deformations.size; j++) {
            deform_t *d = b->deformations.data[j];
            vec3 xa, x;
            matter_to_global_position(b, &x, &d->x0);
            matter_to_local_position(a, &xa, &x);

            double phi = sdf_distance(a->sdf, &xa) + sdf_distance(b->sdf, &d->x0);
            if (phi <= 0) {
                vec3 n = sdf_normal(a->sdf, &xa);
                matter_to_global_direction(a, NULL, &n, &n);
                vec3_multiply_f(&n, &n, -phi * ratio);
                transform_translate(&b->transform, &n);
            }
        }
    }
}

static bool is_colliding_in_bound(substance_t **substances, bound3_t *bound) {
    vec3 radius;
    bound3_radius(bound, &radius);
    if (vec3_length(&radius) <= epsilon) {
        return false;
    }

    bound3_t sub_bounds[2];
    bound3_bisect(bound, sub_bounds);
    double sub_bound_distances[2];

    for (int sub_bound_index = 0; sub_bound_index < 2; sub_bound_index++) {
        vec3 global_position;
        double phis[2];
        bound3_midpoint(&sub_bounds[sub_bound_index], &global_position);

        for (int i = 0; i < 2; i++) {
            vec3 local_position;
            matter_to_local_position(&substances[i]->matter, &local_position,
                                     &global_position);
            phis[i] = sdf_distance(substances[i]->matter.sdf, &local_position);
        }

        sub_bound_distances[sub_bound_index] = phis[0] + phis[1];

        if (sub_bound_distances[sub_bound_index] <= epsilon) {
            return true;
        }

        bound3_radius(&sub_bounds[sub_bound_index], &radius);
        double radius_length = vec3_length(&radius);
        if (phis[0] >= radius_length || phis[1] >= radius_length) {
            return false;
        }

        for (int i = 0; i < 2; i++) {
            substance_t *substance = substances[i];
            if (substance->matter.sdf->is_convex) {
                bool is_intersecting = false;
                vec3 vertex;
                for (int vertex_index = 0; vertex_index < 8; vertex_index++) {
                    bound3_vertex(bound, vertex_index, &vertex);
                    matter_to_local_position(&substance->matter, &vertex, &vertex);
                    double phi = sdf_distance(substance->matter.sdf, &vertex);
                    if (phi < epsilon) {
                        is_intersecting = true;
                        break;
                    }
                }

                if (!is_intersecting) {
                    return false;
                }
            }
        }
    }

    if (sub_bound_distances[0] < sub_bound_distances[1]) {
        return is_colliding_in_bound(substances, &sub_bounds[0]) ||
               is_colliding_in_bound(substances, &sub_bounds[1]);
    } else {
        return is_colliding_in_bound(substances, &sub_bounds[1]) ||
               is_colliding_in_bound(substances, &sub_bounds[0]);
    }
}

static bool collision_narrow_phase(collision_t *c) {
    bound3_t bounds[2];
    for (int matter_index = 0; matter_index < 2; matter_index++) {
        sphere_t *bounding_sphere = &c->substances[matter_index]->bounding_sphere;
        vec3_subtract_f(&bounds[matter_index].lower, &bounding_sphere->c,
                        bounding_sphere->r);
        vec3_add_f(&bounds[matter_index].upper, &bounding_sphere->c,
                   bounding_sphere->r);
    }

    bound3_intersection(&bounds[0], &bounds[1], &c->bound);

    if (!bound3_is_valid(&c->bound)) {
        return false;
    }

    if (is_colliding_in_bound(c->substances, &c->bound)) {
        c->substances[0]->matter.has_collided = true;
        c->substances[1]->matter.has_collided = true;
        return true;
    }

    return false;
}

void collision_detect(substance_t *substance_ptrs, size_t num_substances,
                      collision_array_t *cs, double dt) {
    // clear collisions from last iteration
    for (size_t i = 0; i < cs->size; i++) {
        array_clear(&cs->data[i].manifold);
    }
    array_clear(cs);

    collision_array_t broad_phase_collisions;
    array_create(&broad_phase_collisions);

    collision_broad_phase(substance_ptrs, num_substances, &broad_phase_collisions);

    for (size_t i = 0; i < broad_phase_collisions.size; i++) {
        collision_t *collision = &broad_phase_collisions.data[i];
        if (collision_narrow_phase(collision)) {
            array_push_back(cs);
            *(cs->last) = *collision;
            collision_generate_manifold(cs->last, dt);
        }
    }

    array_clear(&broad_phase_collisions);
}

void collision_resolve(collision_t *self, double dt) {
    collision_resolve_velocity_constraint(self, dt);
    collision_resolve_interpenetration_constraint(self);
}
