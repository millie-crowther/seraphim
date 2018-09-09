#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"

class renderer_t {
private:
    camera_t * main_camera;
    float aspect;

public:
    // constructors and destructors
    renderer_t();
    ~renderer_t();

    // main method
    void render();

    // setters
    void set_main_camera(camera_t * camera);
    void set_aspect_ratio(float aspect);

    // accessors
    mat4_t get_view_matrix();
    mat4_t get_proj_matrix();
};

#endif
