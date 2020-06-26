#include "render/call_and_response.h"

call_t::call_t(){
    index = ~0;  
    status = 0;
}

call_t::call_t(const f32vec3_t & c, float size){
    this->c = c;
    this->size = size;
}

bool 
call_t::is_valid() const {
    return index != static_cast<uint32_t>(~0);
}

f32vec3_t 
call_t::get_centre() const {
    return c;
}

float
call_t::get_size() const {
    return size;
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
    
    if (a.size != b.size){
        return a.size < b.size;
    }

    if ((a.c - b.c).chebyshev_norm() > hyper::epsilon){
        return a.c < b.c;
    } 
   
    return false;  
}

vec3_t vertices[] = {
    vec3_t(-1.0, -1.0, -1.0),
    vec3_t( 1.0, -1.0, -1.0),
    vec3_t(-1.0,  1.0, -1.0),
    vec3_t( 1.0,  1.0, -1.0),
    vec3_t(-1.0, -1.0,  1.0),
    vec3_t( 1.0, -1.0,  1.0),
    vec3_t(-1.0,  1.0,  1.0),
    vec3_t( 1.0,  1.0,  1.0)
};

response_t::response_t(){

}

response_t::response_t(const call_t & call, std::weak_ptr<substance_t> substance_ptr){
    if (auto substance = substance_ptr.lock()){
        vec3_t c = call.get_centre() - substance->get_data().c;
        vec3_t r(call.get_size());

        auto sdf = substance->get_form()->get_sdf();
        uint32_t contains_mask = 0;

        for (int o = 0; o < 8; o++){
            vec3_t d = c + vertices[o].hadamard(r);

            if (!sdf->contains(d)){
                contains_mask |= 1 << (o + 16);
            }

            vec3_t n = sdf->normal(d) / 2 + 0.5;
            normals[o] = squash(vec4_t(n[0], n[1], n[2], 0.0));

            vec3_t c = substance->get_matter()->get_colour(d);
            colours[o] = squash(vec4_t(c[0], c[1], c[2], 0.0));
        }
        
        node = contains_mask | call.get_hash(); 
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

uint32_t
response_t::get_node() const {
    return node;
}

uint32_t
response_t::squash(const vec4_t & x) const {
    u8vec4_t x8 = x * 255;
    return *reinterpret_cast<uint32_t *>(&x8);
}