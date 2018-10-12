#include "prop.h"

#include <chrono>

prop_t::prop_t(){
    
}

void
prop_t::set_mesh(const std::shared_ptr<mesh_t>& mesh){
    this->mesh = mesh;
}

model_t
prop_t::get_model(){
    static auto start = std::chrono::high_resolution_clock::now();
    auto current = std::chrono::high_resolution_clock::now();    
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
        current - start
    ).count();

    mat3_t r = matrix::angle_axis(maths::pi * time / 4.0f, vec3_t({ 0, 1, 0 }));

    transform.set_tf_matrix(mat4_t(r));

    mat4_t m = transform.get_tf_matrix();

    return model_t(m, mesh);
}