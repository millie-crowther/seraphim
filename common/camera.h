#ifndef CAMERA_H
#define CAMERA_H

#include "transform.h"
#include "../frontend/ui.h"

typedef struct camera_t {
    transform_t transform;
} camera_t;

void camera_create(camera_t * camera);
void camera_transformation_matrix(camera_t *camera, float *xs);
void camera_update(camera_t *camera, double delta, const keyboard_t &keyboard, const mouse_t &mouse);

#endif
