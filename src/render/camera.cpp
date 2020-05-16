#include "render/camera.h"

camera_t::camera_t(){
    transform.set_position(vec3_t(0.0, 0.5, -3.0));
}

vec3_t 
camera_t::get_position() const{
    return transform.get_position();
}

vec3_t 
camera_t::get_right() const {
    return transform.get_rotation() * vec3_t::right();
}

vec3_t
camera_t::get_up() const {
    return transform.get_rotation() * vec3_t::up();
}

void 
camera_t::update(double delta, const keyboard_t & keyboard, const mouse_t & mouse){
    vec3_t forward = transform.get_rotation() * vec3_t::forward();

    if (keyboard.is_key_pressed(GLFW_KEY_W)){
        transform.translate(forward * delta);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_S)){
        transform.translate(-forward * delta );
    } 

    if (keyboard.is_key_pressed(GLFW_KEY_A)){
        transform.translate(-get_right() * delta );
    }

    if (keyboard.is_key_pressed(GLFW_KEY_D)){
        transform.translate(get_right() * delta );
    }
    
    transform.rotate(quat_t::angle_axis(
        delta * mouse.get_velocity()[0] / 2000, 
        vec3_t::up()
    ));
}
