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

    painter_t<3> painter;
    const uint8_t s = texture_manager_t::brick_size;
    std::array<colour_t, s * s> image;

    // check if normal already aligned with up vector
    vec3_t u_axis = vec3_t::up();
    if (std::abs(n[1]) > 1.0 - constant::epsilon){
        u_axis = vec3_t::right();
    }

    vec3_t v_axis = u_axis % n;

    vec3_t p0 = x - n * p - u_axis * aabb[3] / 2 - v_axis * aabb[3] / 2;

    for (uint8_t u = 0; u < s; u++){
        for (uint8_t v = 0; v < s; v++){
            image[u + v * s] = painter.colour(
                p0 + 
                u_axis * aabb[3] * u / s + 
                v_axis * aabb[3] * v / s 
            );
        }   
    }

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