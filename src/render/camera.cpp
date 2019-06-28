#include "render/camera.h"

#include "core/blaspheme.h"

camera_t::camera_t(const blaspheme_t * blaspheme){
    transform.set_position(vec3_t(0.0, 0.5, 0.0));
    transform.set_rotation(quat_t::angle_axis(3.14159 / 2, vec3_t::up()));

    const keyboard_t * keyboard = blaspheme->get_keyboard();

    if (auto scheduler = blaspheme->get_scheduler().lock()){
        scheduler->frame_start.follow([keyboard, this](double delta){
            vec3_t forward = transform.get_rotation() * vec3_t::forward();

            if (keyboard->is_key_pressed(GLFW_KEY_W)){
                transform.translate(forward * delta);
            }

            if (keyboard->is_key_pressed(GLFW_KEY_S)){
                transform.translate(-forward * delta );
            }

            if (keyboard->is_key_pressed(GLFW_KEY_A)){
                transform.translate(-get_right() * delta );
            }

            if (keyboard->is_key_pressed(GLFW_KEY_D)){
                transform.translate(get_right() * delta );
            }
        });
    }
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