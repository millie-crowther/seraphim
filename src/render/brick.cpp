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

    std::array<u8vec4_t, texture_manager_t::brick_size * texture_manager_t::brick_size> image;

    vec3_t v = std::abs(n[0]) <= 1 - constant::epsilon ? vec3_t::up() : vec3_t::right();
    vec3_t u_axis = v % n;
    vec3_t v_axis = n % u_axis;

    for (uint32_t i = 0; i < image.size(); i++){
        vec2_t uv(
            i % texture_manager_t::brick_size,
            i / texture_manager_t::brick_size
        );
        uv /= texture_manager_t::brick_size / aabb[3] / 2;

        vec3_t dx = u_axis * uv[0] + v_axis * uv[1];

        vec3_t a = x - n * p + dx;

        image[i] = painter_t<3>().colour(a);
        double pa = sdf.phi(a);
        // if (std::abs(pa) > aabb[3] / 4) std::cout << pa << std::endl;
    }


    if (n[2] < 0){
        n = -n;
        p = -p;
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