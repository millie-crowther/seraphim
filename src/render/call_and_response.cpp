#include "render/call_and_response.h"

using namespace srph;

call_t::call_t(){
    index = ~0;  
    status = 0;
}

bool call_t::is_valid() const {
    return index != static_cast<uint32_t>(~0);
}

f32vec3_t call_t::get_position() const {
    return position;
}

float call_t::get_radius() const {
    return radius;
}

uint32_t call_t::get_index() const {
    return index;
}

uint32_t call_t::get_hash() const {
    return hash;
}

uint32_t call_t::get_substance_ID() const {
    return substanceID;
}

bool call_t::comparator_t::operator()(const call_t & a, const call_t & b) const {
    if (a.substanceID != b.substanceID){
        return a.substanceID < b.substanceID;
    }
    
    if (std::abs(a.radius - b.radius) > constant::epsilon){
        return a.radius < b.radius;
    }

    if (a.position != b.position){
        return f32vec3_t::comparator_t()(a.position, b.position);
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

response_t::response_t(){}

response_t::response_t(const call_t & call, std::weak_ptr<srph_substance> substance_ptr){
    if (auto substance = substance_ptr.lock()){
        srph_sdf * sdf = substance->matter.sdf;

        srph_bound3 * bound = srph_sdf_bound(sdf);
        vec3 m;
        srph_bound3_midpoint(bound, m.v);
        vec3_t p = mat::cast<double>(call.get_position()) - vec3_t(m.x, m.y, m.z);

        uint32_t contains_mask = 0;

        for (int o = 0; o < 8; o++){
            vec3_t d = p + vertices[o] * call.get_radius();
            vec3 d1 = {{ d[0], d[1], d[2] }};

            if (!srph_sdf_contains(sdf, &d1)){
                contains_mask |= 1 << o;
            }

            vec3 n1 = srph_sdf_normal(sdf, &d1);
            vec3_t n = vec3_t(n1.x, n1.y, n1.z) / 2 + 0.5;

            normals[o] = squash(vec4_t(n, 0.0));

            srph_material mat;
            srph_matter_material(&substance->matter, &mat);
            vec3 c = mat.colour;
            colours[o] = squash(vec4_t(c.x, c.y, c.z, 0.0));
        }

        vec3_t c = p + call.get_radius();
        vec3 c1 = {{ c[0], c[1], c[2] }};
        float phi = static_cast<float>(srph_sdf_phi(sdf, &c1));
        
        vec3 n1 = srph_sdf_normal(sdf, &c1);
        vec3_t n = vec3_t(n1.x, n1.y, n1.z) / 2 + 0.5;
        uint32_t np = squash(vec4_t(n, 0.0));

        uint32_t x_elem = contains_mask << 16;
        patch = { x_elem, call.get_hash(), phi, np };    
    }
}

const std::array<uint32_t, 8> & response_t::get_normals() const {
    return normals;
}

const std::array<uint32_t, 8> & response_t::get_colours() const {
    return colours;
}

response_t::patch_t response_t::get_patch() const {
    return patch;
}

uint32_t response_t::squash(const vec4_t & x) const {
    uint8_t bytes[4];
    for (int i = 0; i < 3; i++){
        bytes[i] = (uint8_t)(fmax(0.0, fmin(x[i] * 255.0, 255.0)));
    }
    return *reinterpret_cast<uint32_t *>(bytes);
}
