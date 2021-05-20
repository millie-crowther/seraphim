#ifndef SERAPHIM_TRANSFORM_H
#define SERAPHIM_TRANSFORM_H

#include "maths.h"

typedef struct transform_t {
    vec3 position;
    quat rotation;
} transform_t;

void transform_to_local_position(transform_t *tf, vec3 *tx, const vec3 *x);
void transform_to_global_position(const transform_t *tf, vec3 *tx, const vec3 *x);
void transform_to_global_direction(const transform_t *tf, vec3 *tx, const vec3 *x);

void transform_forward(const transform_t *tf, vec3 *x);
void transform_up(const transform_t *tf, vec3 *x);
void transform_right(const transform_t *tf, vec3 *x);

void transform_translate(transform_t *tf, const vec3 *x);
void transform_rotate(transform_t *tf, const quat *q);

void transform_matrix(transform_t *tf, mat4 *xs);

#endif
