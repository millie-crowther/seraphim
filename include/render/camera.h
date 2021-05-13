#ifndef CAMERA_H
#define CAMERA_H

#include "physics/transform.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"

typedef struct camera_t {
    transform_t transform;
    camera_t();

    void update(double delta, const keyboard_t &keyboard, const mouse_t &mouse);
} camera_t;

void camera_transformation_matrix(camera_t *camera, float *xs);

#endif
