#ifndef CAMERA_H
#define CAMERA_H

#include "physics/transform.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"

namespace srph {
    typedef struct camera_t {
        srph_transform transform;
        camera_t();

        void update(double delta, const keyboard_t & keyboard, const mouse_t & mouse);
    } camera_t;
}

void srph_camera_transformation_matrix(srph::camera_t * camera, float * xs);

#endif
