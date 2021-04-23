//
// Created by millie on 14/04/2021.
//

#ifndef SERAPHIM_BROAD_PHASE_H
#define SERAPHIM_BROAD_PHASE_H

#include "core/array.h"
#include "metaphysics/substance.h"
#include "physics/collision.h"

typedef srph_array(collision_t) srph_collision_array;
void srph_broad_phase_collision(srph_substance *substance_ptrs, size_t num_substances, srph_collision_array *cs);

#endif //SERAPHIM_BROAD_PHASE_H
