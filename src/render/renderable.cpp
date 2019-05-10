#include "render/renderable.h"

renderable_t::renderable_t(const sdf_t & sdf, transform_t & master) : sdf(sdf){
    visible = true;
    transform = master.create_servant();
}

bool 
renderable_t::is_visible() const { 
    return visible; 
}