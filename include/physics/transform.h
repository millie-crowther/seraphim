#ifndef SERAPHIM_TRANSFORM_H
#define SERAPHIM_TRANSFORM_H

#include <memory>
#include <mutex>
#include <vector>

#include "maths/maths.h"

typedef struct srph_transform {
    vec3 position;
    quat rotation;
} srph_transform;

void srph_transform_to_local_position(srph_transform * tf, vec3 * tx, const vec3 * x);
void srph_transform_to_global_position(const srph_transform * tf, vec3 * tx, const vec3 * x);
void srph_transform_to_global_direction(const srph_transform * tf, vec3 * tx, const vec3 * x);

void srph_transform_forward(const srph_transform *tf, vec3 *x);
void srph_transform_up(const srph_transform *tf, vec3 *x);
void srph_transform_right(const srph_transform *tf, vec3 *x);

void srph_transform_translate(srph_transform *tf, const vec3 *x);
void srph_transform_rotate(srph_transform * tf, const quat * q);

void srph_transform_matrix(srph_transform * tf, mat4 * xs);

#endif
