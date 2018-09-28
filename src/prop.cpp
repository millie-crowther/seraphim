#include "prop.h"

#include <chrono>

prop_t::prop_t(const std::shared_ptr<mesh_t>& mesh){
    this->mesh = mesh;
}

std::shared_ptr<mesh_t> 
prop_t::get_mesh(){
    return mesh;
}

mat4_t
prop_t::get_model_matrix(){
    static auto start = std::chrono::high_resolution_clock::now();
    auto current = std::chrono::high_resolution_clock::now();    
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
        current - start
    ).count();

    mat3_t r = matrix::angle_axis(maths::pi * time / 4.0f, vec3_t({ 0, 1, 0 }));

    transform.set_tf_matrix(mat4_t(r));

    return transform.get_tf_matrix();
}