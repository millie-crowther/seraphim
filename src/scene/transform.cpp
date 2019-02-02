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
transform_t::get_matrix() const {
    return matrix;
}

void
transform_t::set_parent(transform_t& parent){
    this->parent = &parent;
}

void
transform_t::set_matrix(const mat4_t & matrix){
    this->matrix = matrix;
}