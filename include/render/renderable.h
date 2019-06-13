#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "sdf/sdf.h"
#include "scene/transform.h"
#include "core/aabb.h"

struct renderable_t {
    std::shared_ptr<sdf3_t> sdf;
    std::shared_ptr<transform_t> transform;

    // constructors
    renderable_t(std::shared_ptr<sdf3_t> sdf, transform_t & master);

    // predicates
    bool intersects(const aabb_t & aabb) const;
    bool contains(const aabb_t & aabb) const;

};

#endif
