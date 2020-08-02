#include "render/camera.h"

camera_t::camera_t(){
    transform.set_position(vec3_t(0.0, 0.5, -3.0));
}

void 
camera_t::update(double delta, const keyboard_t & keyboard, const mouse_t & mouse){
    vec3_t forward = transform.get_rotation() * vec3_t::forward();
    vec3_t right = transform.get_rotation() * vec3_t::right();

    if (keyboard.is_key_pressed(GLFW_KEY_W)){
        transform.translate(forward * delta);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_S)){
        transform.translate(-forward * delta );
    } 

    if (keyboard.is_key_pressed(GLFW_KEY_A)){
        transform.translate(-right * delta );
    }

    if (keyboard.is_key_pressed(GLFW_KEY_D)){
        transform.translate(right * delta );
    }
    
    transform.rotate(quat_t::angle_axis(
        delta * mouse.get_velocity()[0] / 2000, 
        vec3_t::up()
    ));
}

f32mat4_t 
camera_t::get_matrix(){
    return *transform.get_matrix();
}