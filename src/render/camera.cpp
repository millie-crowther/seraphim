#include "render/camera.h"

#include <assert.h>
#include <math.h>

void camera_transformation_matrix(camera_t *camera, float *xs) {
    assert(camera != NULL && xs != NULL);
    mat4 dxs;
    transform_matrix(&camera->transform, &dxs);

    for (int i = 0; i < MAT4_SIZE; i++) {
        xs[i] = dxs.v[i];
    }
}

camera_t::camera_t() {
    transform.position = {{0.0, 0.5, -5.0}};
    transform.rotation = quat_identity;
}

void camera_update(camera_t *camera, double delta, const keyboard_t &keyboard, const mouse_t &mouse) {
    double scale = 2000.0;

    quat q;
    quat_from_axis_angle(&q, &vec3_up, delta * mouse.velocity.x / scale);
    transform_rotate(&camera->transform, &q);

    vec3 right;
    quat q1;
    transform_right(&camera->transform, &right);
    right.y = 0;
    vec3_normalize(&right, &right);
    quat_from_axis_angle(&q1, &right, delta * mouse.velocity.y / scale);
    transform_rotate(&camera->transform, &q1);

    vec3 forward;
    transform_forward(&camera->transform, &forward);
    forward.y = 0.0;
    vec3_normalize(&forward, &forward);

    vec3 move_right;
    vec3_cross(&move_right, &forward, &vec3_up);

    vec3 d;
    if (keyboard.is_key_pressed(GLFW_KEY_W)) {
        vec3_multiply_f(&d, &forward, delta);
        transform_translate(&camera->transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_S)) {
        vec3_multiply_f(&d, &forward, -delta);
        transform_translate(&camera->transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_A)) {
        vec3_multiply_f(&d, &move_right, delta);
        transform_translate(&camera->transform, &d);
    }

    if (keyboard.is_key_pressed(GLFW_KEY_D)) {
        vec3_multiply_f(&d, &move_right, -delta);
        transform_translate(&camera->transform, &d);
    }
}