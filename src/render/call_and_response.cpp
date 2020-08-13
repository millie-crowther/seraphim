#include "render/call_and_response.h"

call_t::call_t(){
    index = ~0;  
    status = 0;
}

bool 
call_t::is_valid() const {
    return index != static_cast<uint32_t>(~0);
}

f32vec3_t 
call_t::get_position() const {
    return position;
}

float
call_t::get_radius() const {
    return radius;
}

uint32_t 
call_t::get_index() const {
    return index;
}

uint32_t
call_t::get_hash() const {
    return hash;
}

uint32_t
call_t::get_substance_ID() const {
    return substanceID;
}

bool
call_t::comparator_t::operator()(const call_t & a, const call_t & b) const {
    if (a.substanceID != b.substanceID){
        return a.substanceID < b.substanceID;
    }
    
    if (std::abs(a.radius - b.radius) > hyper::epsilon){
        return a.radius < b.radius;
    }

    if ((a.position - b.position).chebyshev_norm() > hyper::epsilon){
        return a.position < b.position;
    } 
   
    return false;  
}

vec3_t vertices[8] = {
    vec3_t(0.0, 0.0, 0.0),
    vec3_t(2.0, 0.0, 0.0),
    vec3_t(0.0, 2.0, 0.0),
    vec3_t(2.0, 2.0, 0.0),
    vec3_t(0.0, 0.0, 2.0),
    vec3_t(2.0, 0.0, 2.0),
    vec3_t(0.0, 2.0, 2.0),
    vec3_t(2.0, 2.0, 2.0)
};

response_t::response_t(){

}

response_t::response_t(const call_t & call, std::weak_ptr<substance_t> substance_ptr){
    if (auto substance = substance_ptr.lock()){
        auto sdf = substance->get_matter()->get_sdf();
        vec3_t p = call.get_position() - sdf->get_aabb().get_centre();

        uint32_t contains_mask = 0;

        for (int o = 0; o < 8; o++){
            vec3_t d = p + vertices[o] * call.get_radius();

            if (!sdf->contains(d)){
                contains_mask |= 1 << o;
            }

            vec3_t n = sdf->normal(d) / 2 + 0.5;
            normals[o] = squash(vec4_t(n[0], n[1], n[2], 0.0));

            vec3_t c = substance->get_matter()->get_material(d).colour;
            colours[o] = squash(vec4_t(c[0], c[1], c[2], 0.0));
        }

        uint32_t x_elem = contains_mask << 16;
        node = u32vec2_t(x_elem, call.get_hash()); 
    }
}

const std::array<uint32_t, 8> &
response_t::get_normals() const {
    return normals;
}

const std::array<uint32_t, 8> &
response_t::get_colours() const {
    return colours;
}

u32vec2_t
response_t::get_node() const {
    return node;
}

uint32_t
response_t::squash(const vec4_t & x) const {
    u8vec4_t x8 = x * 255;
    return *reinterpret_cast<uint32_t *>(&x8);
}