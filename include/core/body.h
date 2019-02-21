#ifndef BODY_H
#define BODY_H

#include <memory>

#include "render/renderable.h"
#include "physics/rigidbody.h"

class body_t {
private:
    sdf_t sdf;

    renderable_t renderable;
    collider_t collider;

public:
};

#endif