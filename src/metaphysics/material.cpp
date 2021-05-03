//
// Created by millie on 02/05/2021.
//

#include "metaphysics/material.h"

void material_create(material_t *material, uint32_t *id, const vec3 *colour) {
    material->static_friction = 0.2;
    material->dynamic_friction = 0.1;
    material->density = 700.0;
    material->restitution = 0.5;
    material->colour = *colour;

    material->id = *id;
    (*id)++;
}
