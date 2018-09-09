#include "transform.h"

transform_t::transform_t(){
    parent == nullptr;
    tf = mat4_t::identity();
}

mat4_t
transform_t::get_tf_matrix(){
    return tf;
}

void
transform_t::set_tf_matrix(const mat4_t& m){
    tf = m;
}