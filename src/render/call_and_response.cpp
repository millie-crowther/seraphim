#include "render/call_and_response.h"

call_t::call_t(){
    child = 0;  
    status = 0;
}

uint32_t
call_t::get_substance_ID() const {
    return substanceID;
}

response_t::response_t(const call_t & call, std::weak_ptr<substance_t> substance_ptr){
    if (auto substance = substance_ptr.lock()){
        vec3_t c = call.c - substance->get_data().c;
        vec3_t r = substance->get_data().r / (1 << call.depth);

        for (int o = 0; o < 8; o++){
            vec3_t d = (vec3_t((o & 1) << 1, o & 2, (o & 4) >> 1)  - 1).hadamard(r);
                
            if (auto sdf = substance->get_sdf().lock()){
                // create normals
                vec3_t n = sdf->normal(c + d) / 2 + 0.5;
                normals[o] = squash(vec4_t(n[0], n[1], n[2], 0.0));
                
                // create octree node
                octree_node_t new_node(c + d / 2, r / 2, c + d, sdf);
                nodes[o] = u32vec2_t(new_node.header, new_node.geometry);
            }

            if (auto matter = substance->get_matter().lock()){
                vec3_t c = matter->get_colour(c + d) * 255.0;
                colours[o] = squash(vec4_t(c[0], c[1], c[2], 0.0));
            }
        }
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

std::array<octree_node_t, 8>
response_t::get_nodes() const {
    std::array<octree_node_t, 8> r;
    for (int i = 0; i < 8; i++){
        r[i].header = nodes[i][0];
        r[i].geometry = nodes[i][1];
    }
    return r;
}

uint32_t
response_t::squash(const vec4_t & x) const {
    u8vec4_t x8 = x * 255;
    return *reinterpret_cast<uint32_t *>(&x8);
}
