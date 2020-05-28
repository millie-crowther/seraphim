#include "render/call_and_response.h"

call_t::call_t(){
    child = 0;  
    status = 0;
}

call_t::call_t(const f32vec3_t & c, float size){
    this->c = c;
    this->size = size;
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
call_t::get_child() const {
    return child;
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

constexpr uint32_t response_t::node_unused_flag;

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

        for (int o = 0; o < 8; o++){
            vec3_t d = vertices[o].hadamard(r);
                
            auto sdf = substance->get_form()->get_sdf();
            // create normals
            vec3_t n = sdf->normal(c + d) / 2 + 0.5;
            normals[o] = squash(vec4_t(n[0], n[1], n[2], 0.0));
            
            // create octree node
            nodes[o] = create_node(c + d / 2, r / 2, sdf);

            vec3_t c = substance->get_matter()->get_colour(c + d);
            colours[o] = squash(vec4_t(c[0], c[1], c[2], 0.0));
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

const std::array<u32vec2_t, 8> &
response_t::get_nodes() const {
    return nodes;
}

uint32_t
response_t::squash(const vec4_t & x) const {
    u8vec4_t x8 = x * 255;
    return *reinterpret_cast<uint32_t *>(&x8);
}

bool
response_t::intersects(const vec3_t & c, const vec3_t & r, std::shared_ptr<sdf3_t> sdf) const {
    double lower_radius = r.chebyshev_norm();
    double upper_radius = r.norm();

    double p = sdf->phi(c);

    // 1. is aabb definitely fully inside SDF?
    if (p <= -upper_radius){
        return false;
    }

    // 2. is it possible that part of aabb is partially outside SDF?
    if (std::abs(p) <= lower_radius){
        return true;
    }

    // 3. is aabb definitely fully outside SDF?
    if (p >= upper_radius){
        return false;
    }

    // 4. same as test 1 but more expensive and precise
    double d = (sdf->normal(c) * p).chebyshev_norm();
    if (p < 0 && d > lower_radius){
        return false;
    }

    // 5. same as test 2 but more precise (again, i think)
    if (d <= lower_radius){
        return true;
    }

    // 6. default case
    return false;
}

u32vec2_t
response_t::create_node(const vec3_t & c, const vec3_t & r, std::shared_ptr<sdf3_t> sdf) const {
    double p = sdf->phi(c);
    p /= r.norm() * 2;
    p += 0.5;
    p *= 255;
    p = std::max(0.0, std::min(p, 255.0));

    vec3_t n = (sdf->normal(c) / 2 + 0.5) * 255;
    u8vec4_t normal = vec4_t(n[0], n[1], n[2], p).cast<uint8_t>();
    
    return u32vec2_t(
        intersects(c, r, sdf) ? 0 : node_empty_flag,
        *reinterpret_cast<uint32_t *>(&normal)
    );
}
