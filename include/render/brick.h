#ifndef BRICK_H
#define BRICK_H

#include <vector>
#include <memory>

#include "renderable.h"
#include "aabb.h"

class brick_t {
private:
    std::vector<std::weak_ptr<renderable_t>> renderables;
    aabb_t aabb;

public:
    brick_t(
        const aabb_t & aabb, 
        const std::vector<std::weak_ptr<renderable_t>> & renderables
    );

    // TODO: serialisation function for bricks. voxels? polynomial fit?
};

#endif