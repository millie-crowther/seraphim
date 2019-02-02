#include "render/camera.h"

#include <iostream>

#include "logic/scheduler.h"

camera_t::camera_t(float fov, float near, float far){
    this->fov = fov;
    this->near = near;
    this->far = far;

    transform.set_matrix(
        matrix::look_at(vec3_t(-2), vec3_t(0), vec3_t({0, 1, 0}))
    );

    scheduler::submit([&](){
        std::cout << "hewwo" << std::endl;
    });    
}

transform_t *
camera_t::get_transform(){
    return &transform;
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
