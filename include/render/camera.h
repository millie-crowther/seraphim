#ifndef CAMERA_H
#define CAMERA_H

#include "scene/transform.h"

class blaspheme_t;

class camera_t {
private:
    transform_t transform;

public:
    camera_t(const blaspheme_t * engine);

    // accessors
    vec3_t get_position() const;
    vec3_t get_right() const;
    vec3_t get_up() const;
};

#endif