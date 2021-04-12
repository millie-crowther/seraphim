#include "render/camera.h"

#include <assert.h>
#include <math.h>

using namespace srph;

camera_t::camera_t(){
    transform.position = {{0.0, 0.5, -5.0}};
    transform.rotation = quat_identity;
}

void camera_t::update(double delta, const keyboard_t & keyboard, const mouse_t & mouse){
    double scale = 2000.0;

    quat q;
    quat_from_axis_angle(
        &q,
        &vec3_up,
        delta * mouse.get_velocity()[0] / scale
    );
    srph_transform_rotate(&transform, &q);


    vec3 right;
    quat q1;
    srph_transform_right(&transform, &right);
    right.y = 0;
    vec3_normalize(&right, &right);
    quat_from_axis_angle(
        &q1,
        &right,
        delta * mouse.get_velocity()[1] / scale
    );
    srph_transform_rotate(&transform, &q1);

    vec3 forward;
    srph_transform_forward(&transform, &forward);
    forward.y = 0.0;
    vec3_normalize(&forward, &forward);

    vec3 move_right;
    vec3_cross(&move_right, &forward, &vec3_up);

    vec3 d;
    if (keyboard.is_key_pressed(GLFW_KEY_W)){
        vec3_multiply_f(&d, &forward, delta);
        srph_transform_translate(&transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_S)){
        vec3_multiply_f(&d, &forward, -delta);
        srph_transform_translate(&transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_A)){
        vec3_multiply_f(&d, &move_right, delta);
        srph_transform_translate(&transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_D)){
        vec3_multiply_f(&d, &move_right, -delta);
        srph_transform_translate(&transform, &d);
    }

    assert(isfinite(delta));
    assert(isfinite(mouse.get_velocity()[0]));
    assert(isfinite(mouse.get_velocity()[1]));

}

void srph_camera_transformation_matrix(srph::camera_t *c, float *xs) {
    assert(c != NULL && xs != NULL);
    mat4 dxs;
    srph_transform_matrix(&c->transform, &dxs);

    for (int i = 0; i < 16; i++){
        xs[i] = (float) dxs.v[i];
    }
}
