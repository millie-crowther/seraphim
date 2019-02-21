#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "core/sdf.h"
#include "scene/transform.h"

class renderable_t {
private:
    bool visible;
    sdf_t sdf;
    std::shared_ptr<transform_t> transform;
    
public:
    // constructors
    renderable_t(transform_t & parent);

    // getters
    bool is_visible() const;
};

#endif
