#include "render/renderable.h"

renderable_t::renderable_t(transform_t & parent, const std::shared_ptr<mesh_t>& mesh){
    this->mesh = mesh;
    visible = true;
    transform.set_parent(parent);
}

bool 
renderable_t::is_visible(){ 
    return visible; 
}

std::shared_ptr<mesh_t> 
renderable_t::get_mesh(){
    return mesh;
}

mat4_t 
renderable_t::get_matrix(){
    return transform.get_matrix();
}