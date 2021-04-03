#include "render/camera.h"

#include <assert.h>

#include "maths/vector.h"

using namespace srph;

camera_t::camera_t(){
    transform.position = {{{0.0, 0.5, -5.0}}};
    transform.rotation = srph_quat_identity;
}

void camera_t::update(double delta, const keyboard_t & keyboard, const mouse_t & mouse){
    vec3 forward;
    srph_transform_forward(&transform, &forward);
    forward.y = 0.0;
    srph_vec3_normalise(&forward, &forward);

    vec3 right;
    srph_transform_right(&transform, &right);

    vec3 d;
    if (keyboard.is_key_pressed(GLFW_KEY_W)){
        srph_vec3_scale(&d, &forward, delta);
        srph_transform_translate(&transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_S)){
        srph_vec3_scale(&d, &forward, -delta);
        srph_transform_translate(&transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_A)){
        srph_vec3_scale(&d, &right, -delta);
        srph_transform_translate(&transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_D)){
        srph_vec3_scale(&d, &right, delta);
        srph_transform_translate(&transform, &d);
    }

    double scale = 2000.0;

    srph_quat q;
    srph_quat_angle_axis(
        &q,
        -delta * mouse.get_velocity()[0] / scale,
        &srph_vec3_up
    );
    srph_transform_rotate(&transform, &q);

    srph_transform_right(&transform, &right);
    srph_quat_angle_axis(
        &q,
        -delta * mouse.get_velocity()[1] / scale,
        &right
    );
    srph_transform_rotate(&transform, &q);
}

void srph_camera_transformation_matrix(srph::camera_t *c, float *xs) {
    assert(c != NULL && xs != NULL);
    double dxs[16];
    srph_transform_matrix(&c->transform, dxs);

    for (int i = 0; i < 16; i++){
        xs[i] = (float) dxs[i];
    }
}
