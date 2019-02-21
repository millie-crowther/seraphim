#ifndef BODY_H
#define BODY_H

#include <memory>

#include "render/renderable.h"
#include "physics/rigidbody.h"
#include "material.h"

class body_t {
private:
    sdf_t sdf;
    material_t material;

    renderable_t renderable;
    collider_t collider;
    std::unique_ptr<rigidbody_t> rigidbody;

public:
};

#endif