//
// Created by millie on 14/04/2021.
//

#include "physics//broad_phase.h"

static int axis_comparator(const srph_substance *a, const srph_substance *b, int axis) {
    const srph_sphere *sa = &a->matter.bounding_sphere;
    const srph_sphere *sb = &b->matter.bounding_sphere;

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
    return axis_comparator((const srph_substance *) a, (const srph_substance *) b, X_AXIS);
}

void srph_broad_phase_collision(const srph_broad_phase *phase, srph_collision_array *cs) {
    srph_array_clear(cs);
    srph_array_insertion_sort(&phase->x, x_comparator);

    for (size_t i = 0; i < phase->x.size; i++){
        srph_substance * a = phase->x.data[i];
        srph_sphere * sa = &a->matter.bounding_sphere;

        for (size_t j = i + 1; j < phase->x.size; i++){
            srph_substance * b = phase->x.data[j];
            srph_sphere * sb = &b->matter.bounding_sphere;

            if (sa->c.x + sa->r < sb->c.x - sb->r){
                break;
            }

            if (srph_matter_is_at_rest(&a->matter) && srph_matter_is_at_rest(&b->matter)){
                continue;
            }

            srph_array_push_back(cs);
            *(cs->last) = {
                .ms = { &a->matter, &b->matter },
                .x = vec3_zero,
                .is_colliding = false,
            };
        }
    }
}

void srph_broad_phase_init(srph_broad_phase *self) {
    srph_array_init(&self->x);
}

void srph_broad_phase_destroy(srph_broad_phase *self) {
    srph_array_clear(&self->x);
}
