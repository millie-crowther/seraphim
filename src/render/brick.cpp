#include "render/brick.h"

#include "render/painter.h"
#include "core/constant.h"

brick_t::brick_t(
    const vec4_t & aabb, 
    std::weak_ptr<texture_manager_t> texture_manager_ptr, 
    const sdf3_t & sdf, 
    data_t * data
){
    vec3_t x = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);
    vec3_t n = sdf.normal(x);
    double p = sdf.phi(x);

    if (n[2] < 0){
        n = -n;
        p = -p;
    }

    colour_t colour = painter_t<3>().colour(x - n * p);
    std::array<colour_t, texture_manager_t::brick_size * texture_manager_t::brick_size> image;
    image.fill(colour);

    if (data != nullptr){
        data->n = f32vec2_t(static_cast<float>(n[0]), static_cast<float>(n[1]));
        data->d = (x * n) - p;

        if (auto texture_manager = texture_manager_ptr.lock()){
            u16vec2_t uv = texture_manager->request(image);
            data->uv = uv;
            this->uv = uv;
        }
    }
}

brick_t::~brick_t(){
    if (auto texture_manager = texture_manager_ptr.lock()){
        texture_manager->clear(uv);
    }
}

bool 
brick_t::operator<(const brick_t & brick) const {
    return uv < brick.uv;
}