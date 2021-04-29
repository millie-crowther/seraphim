#ifndef SERAPHIM_TRANSFORM_H
#define SERAPHIM_TRANSFORM_H

#include <memory>
#include <mutex>
#include <vector>

#include "maths/maths.h"

typedef struct transform_t {
	vec3 position;
	quat rotation;
} transform_t;

void srph_transform_to_local_position(transform_t * tf, vec3 * tx, const vec3 * x);
void srph_transform_to_global_position(const transform_t * tf, vec3 * tx,
	const vec3 * x);
void srph_transform_to_global_direction(const transform_t * tf, vec3 * tx,
	const vec3 * x);

void srph_transform_forward(const transform_t * tf, vec3 * x);
void srph_transform_up(const transform_t * tf, vec3 * x);
void srph_transform_right(const transform_t * tf, vec3 * x);

void srph_transform_translate(transform_t * tf, const vec3 * x);
void srph_transform_rotate(transform_t * tf, const quat * q);

void srph_transform_matrix(transform_t * tf, mat4 * xs);

#endif
