#ifndef CAMERA_H
#define CAMERA_H

#include "physics/transform.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"

class camera_t {
private:
    transform_t transform;

public:
    camera_t();

    void update(double delta, const keyboard_t & keyboard, const mouse_t & mouse);

    f32mat4_t get_matrix();
    vec3_t get_position() const;
};

#endif
