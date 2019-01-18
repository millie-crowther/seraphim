#include "scene/transform.h"

transform_t::transform_t() {
    this->parent = nullptr;
    matrix = mat4_t::identity();
}

transform_t::transform_t(transform_t & parent){
    this->parent = &parent;
    matrix = mat4_t::identity();
}

mat4_t
transform_t::get_matrix(){
    return matrix;
}

void
transform_t::set_parent(transform_t& parent){
    this->parent = &parent;
}