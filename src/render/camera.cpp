#include "render/camera.h"

#include <iostream>

#include "logic/scheduler.h"

camera_t::camera_t(float fov, float near, float far){
    this->fov = fov;
    this->near = near;
    this->far = far;
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
