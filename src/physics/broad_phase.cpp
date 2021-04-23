//
// Created by millie on 14/04/2021.
//

#include "physics//broad_phase.h"

static int axis_comparator(const srph_substance *a, const srph_substance *b, int axis) {
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
    return axis_comparator(*(const srph_substance **) a, *(const srph_substance **) b, X_AXIS);
}


void srph_broad_phase_collision(srph_substance *substance_ptrs, size_t num_substances, srph_collision_array *cs) {
    srph_array_clear(cs);

    srph_array(srph_substance *) substances{};
    srph_array_init(&substances);

    for (size_t i = 0; i < num_substances; i++) {
        srph_array_push_back(&substances);
        substances.data[i] = &substance_ptrs[i];
    }

    srph_array_sort(&substances, x_comparator);

    for (size_t i = 0; i < num_substances; i++) {
        srph_matter *a = &substances.data[i]->matter;
        sphere_t *sa = &a->bounding_sphere;

        for (size_t j = i + 1; j < num_substances; j++) {
            srph_matter *b = &substances.data[j]->matter;
            sphere_t *sb = &b->bounding_sphere;

            if (sa->c.x + sa->r < sb->c.x - sb->r) {
                break;
            }

            if (srph_matter_is_at_rest(a) && srph_matter_is_at_rest(b)) {
                continue;
            }

            srph_array_push_back(cs);
            *(cs->last) = {
                .ms = {a, b},
//                    .x = vec3_zero,
                .is_colliding = false,
            };
        }
    }
}
