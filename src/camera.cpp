#include "camera.h"

camera_t::camera_t(float fov, float near, float far){
    this->fov = fov;
    this->near = near;
    this->far = far;
}

transform_t *
camera_t::get_transform(){
    return &tf;
}

float
camera_t::get_fov(){
    return fov;
}

float 
camera_t::get_near(){
    return near;
}

float 
camera_t::get_far(){
    return far;
}