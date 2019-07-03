#include "render/brick.h"

#include "render/painter.h"
#include "core/hyper.h"

brick_t::brick_t(
    const vec4_t & aabb, 
    std::weak_ptr<texture_manager_t> texture_manager_ptr, 
    const sdf3_t & sdf
){
    vec3_t c = vec3_t(aabb[0], aabb[1], aabb[2]) + vec3_t(aabb[3] / 2);

    u8vec4_t colour = painter_t<3>().colour(c);
    
    vec3_t n = (sdf.normal(c) / 2 + 0.5) * 255;
    u8vec4_t normal(n[0], n[1], n[2], 0);

    if (auto texture_manager = texture_manager_ptr.lock()){
        id = texture_manager->request(colour, normal);
        this->id = id;
    }
}

brick_t::~brick_t(){
    if (auto texture_manager = texture_manager_ptr.lock()){
        texture_manager->clear(id);
    }
}

bool 
brick_t::operator<(const brick_t & brick) const {
    return id < brick.id;
}

uint32_t
brick_t::get_id() const {
    return id;
}