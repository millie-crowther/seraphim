#ifndef SERAPHIM_MATERIAL_H
#define SERAPHIM_MATERIAL_H

#include "maths/maths.h"

typedef struct material_t {
    vec3 colour;
    double density;
    double restitution;
    double static_friction;
    double dynamic_friction;
    double metallic;
    double roughness;
    double reflectance;
    uint32_t id;
} material_t;

void material_create(material_t *material, uint32_t id, const vec3 *colour);
void material_colour(material_t *material, const vec3 * x, vec3 * colour);
void material_physical(material_t *material, const vec4 * x, vec4 *physical);

#endif
