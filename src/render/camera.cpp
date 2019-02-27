#include "render/camera.h"

#include <iostream>

#include "logic/scheduler.h"

camera_t::camera_t(float fov, float near, float far){
    this->fov = fov;
    this->near = near;
    this->far = far;

    transform.set_position(vec3_t(2));
    transform.set_rotation(quat_t::look_at(vec3_t(-1).normalise(), vec3_t(0, 1, 0)));
}

float
camera_t::get_fov() const {
    return fov;
}

float 
camera_t::get_near() const {
    return near;
}

float 
camera_t::get_far() const {
    return far;
}
