#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <memory>
#include <vector>

#include "maths/quat.h"

class transform_t {
private:
    // master and servant hierarchy
    transform_t * master;
    std::vector<std::shared_ptr<transform_t>> servants;

    // physical location and orientation
    vec3_t position;
    quat_t rotation;

public:
    // constructors
    transform_t();

    // modifiers
    std::weak_ptr<transform_t> create_servant();
};

#endif
