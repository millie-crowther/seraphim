#include "scene/transform.h"

transform_t::transform_t() {
    this->parent = nullptr;
}


transform_t::transform_t(transform_t * parent){
    this->parent = parent;
}

std::shared_ptr<transform_t> 
transform_t::add_child(){
    std::shared_ptr<transform_t> child(this);
    children.push_back(child);
    return child;
}