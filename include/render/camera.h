#ifndef CAMERA_H
#define CAMERA_H

#include "scene/transform.h"

class camera_t {
private:
    // private fields
    float fov;
    float near;
    float far;

public:
    // constructors and destructors
    camera_t(float fov, float near, float far);

    // getters
    float get_fov() const;
    float get_near() const;
    float get_far() const;
};

#endif