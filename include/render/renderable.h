#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "core/sdf.h"
#include "scene/transform.h"
#include "aabb.h"

class renderable_t {
private:
    bool visible;
    sdf_t sdf;
    std::shared_ptr<transform_t> transform;
    
public:
    // constructors
    renderable_t(const sdf_t & sdf, transform_t & master);

    // predicates
    bool is_visible() const;
    bool intersects(const aabb_t & aabb) const;
    bool contains(const aabb_t & aabb) const;
};

#endif
