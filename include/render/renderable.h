#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "sdf/sdf.h"
#include "scene/transform.h"
#include "core/aabb.h"

class renderable_t {
private:
    bool visible;
    std::shared_ptr<sdf_t> sdf;
    std::shared_ptr<transform_t> transform;
    
public:
    // constructors
    renderable_t(std::shared_ptr<sdf_t> sdf, transform_t & master);

    // predicates
    bool is_visible() const;
    bool intersects(const aabb_t & aabb) const;

    // accessors
    vec4_t plane(const vec3_t & x) const;
};

#endif
