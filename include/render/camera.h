#ifndef CAMERA_H
#define CAMERA_H

#include "logic/interval_revelator.h"
#include "scene/transform.h"

class blaspheme_t;

class camera_t {
private:
    transform_t transform;

    interval_revelator_t::follower_ptr_t frame_start_follower;

public:
    camera_t(const blaspheme_t * engine);

    // accessors
    vec3_t get_position() const;
    vec3_t get_right() const;
    vec3_t get_up() const;
};

#endif