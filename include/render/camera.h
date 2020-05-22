#ifndef CAMERA_H
#define CAMERA_H

#include "scene/transform.h"
#include "ui/keyboard.h"
#include "ui/mouse.h"

class blaspheme_t;

class camera_t {
private:
    transform_t transform;

public:
    camera_t();

    void update(double delta, const keyboard_t & keyboard, const mouse_t & mouse);

    // accessors
    vec3_t get_position() const;
    vec3_t get_right() const;
    vec3_t get_up() const;
    vec3_t get_forward() const;
};

#endif