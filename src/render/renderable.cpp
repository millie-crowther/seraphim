#include "render/renderable.h"

renderable_t::renderable_t(transform_t & parent){
    visible = true;
    transform = parent.add_child();
}

bool 
renderable_t::is_visible() const { 
    return visible; 
}