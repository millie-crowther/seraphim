//
// Created by millie on 14/04/2021.
//

#ifndef SERAPHIM_BROAD_PHASE_H
#define SERAPHIM_BROAD_PHASE_H

#include "core/array.h"
#include "metaphysics/substance.h"
#include "physics/collision.h"

typedef srph_array(srph_collision) srph_collision_array;
typedef srph_array(srph_substance *) srph_substance_ptr_array;

typedef struct srph_broad_phase {
    srph_substance_ptr_array x;
} srph_broad_phase;

void srph_broad_phase_init(srph_broad_phase *self);
void srph_broad_phase_destroy(srph_broad_phase *self);
void srph_broad_phase_collision(const srph_broad_phase *phase, srph_collision_array *cs);

#endif //SERAPHIM_BROAD_PHASE_H
