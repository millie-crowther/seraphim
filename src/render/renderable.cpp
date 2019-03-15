#include "render/renderable.h"

renderable_t::renderable_t(transform_t & master){
    visible = true;
    transform = master.create_servant();
}

bool 
renderable_t::is_visible() const { 
    return visible; 
}