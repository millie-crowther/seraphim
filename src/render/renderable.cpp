#include "render/renderable.h"

renderable_t::renderable_t(const std::shared_ptr<mesh_t>& mesh){
    this->mesh = mesh;
    visible = true;
}

bool 
renderable_t::is_visible(){ 
    return visible; 
}

std::shared_ptr<mesh_t> 
renderable_t::get_mesh(){
    return mesh;
}