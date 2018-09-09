#include "renderer.h"

renderer_t::renderer_t(){
    main_camera = new camera_t(maths::to_radians(45.0f), 0.1f, 10.0f);
}

renderer_t::~renderer_t(){
    delete main_camera; //TODO delete
}

void
renderer_t::render(){

}

void
renderer_t::set_main_camera(camera_t * camera){
    main_camera = camera;
}

mat4_t
renderer_t::get_view_matrix(){
    return main_camera->get_transform()->get_tf_matrix();
}

mat4_t
renderer_t::get_proj_matrix(){
    return matrix::perspective(
        main_camera->get_fov(),
        aspect,
        main_camera->get_near(),
        main_camera->get_far()
    );
}

void
renderer_t::set_aspect_ratio(float aspect){
    this->aspect = aspect;
}