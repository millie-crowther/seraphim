#include "render/brick.h"

brick_t::brick_t(
    const vec3_t & x, 
    std::weak_ptr<texture_manager_t> texture_manager_ptr, 
    std::weak_ptr<sdf_t> sdf_ptr, 
    data_t * data
){
    if (auto sdf = sdf_ptr.lock()){
        vec3_t n = sdf->normal(x);
        double p = sdf->phi(x);

        if (n[2] < 0){
            n = -n;
            p = -p;
        }

        if (data != nullptr){
            data->n = f32vec2_t(n[0], n[1]);
            data->d = (x * n) - p;

            if (auto texture_manager = texture_manager_ptr.lock()){
                u16vec2_t uv = texture_manager->request();
                data->uv = uv;
                this->uv = uv;
            }
        }
    }
}

~brick_t(){
    if (auto texture_manager = texture_manager_ptr.lock()){
        texture_manager_ptr->clear(uv);
    }
}