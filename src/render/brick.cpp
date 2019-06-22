#include "render/brick.h"

brick_t::brick_t(){}

brick_t::brick_t(
    const vec3_t & x, 
    std::weak_ptr<texture_manager_t> texture_manager_ptr, 
    const sdf3_t & sdf, 
    data_t * data
){
    // if (auto sdf = sdf_ptr.lock()){
        vec3_t n = sdf.normal(x);
        double p = sdf.phi(x);

        if (n[2] < 0){
            n = -n;
            p = -p;
        }

        if (data != nullptr){
            data->n = f32vec2_t(static_cast<float>(n[0]), static_cast<float>(n[1]));
            data->d = static_cast<float>((x * n) - p);

            // if (auto texture_manager = texture_manager_ptr.lock()){
            //     u16vec2_t uv = texture_manager->request();
            //     data->uv = uv;
            //     this->uv = uv;
            // }
        }
    // }
}

brick_t::~brick_t(){
    // if (auto texture_manager = texture_manager_ptr.lock()){
    //     texture_manager_ptr->clear(uv);
    // }
}