#ifndef CAMERA_H
#define CAMERA_H

#include "transform.h"

class camera_t {
private:
    // private fields
    transform_t tf;
    float fov;
    float near;
    float far;

public:
    // constructors and destructors
    camera_t(float fov, float near, float far);

    // getters
    transform_t * get_transform();
    float get_fov();
    float get_near();
    float get_far();
    
};

#endif