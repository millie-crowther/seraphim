#include "substance/matter/matter.h"

matter_t::data_t::data_t(){
    id = ~0;
}

matter_t::data_t::data_t(float near, float far, const f32vec3_t & r, uint32_t id, const f32mat4_t & transform){
    this->near = near;
    this->far = far;
    this->r = r;
    this->id = id;
    this->transform = transform;
}

matter_t::matter_t(std::shared_ptr<sdf3_t> sdf, const vec3_t & colour){
    static uint32_t id = 0;

    this->sdf = sdf;
    this->colour = colour;
    this->id = id++;
}

bool 
matter_t::data_t::comparator_t::operator()(const matter_t::data_t & a, const matter_t::data_t & b) const {
    return a.far < b.far && a.id != static_cast<uint32_t>(~0);
}

std::shared_ptr<sdf3_t>
matter_t::get_sdf() const {
    return sdf;
}

vec3_t
matter_t::get_colour(const vec3_t & x) const {
    return colour;
}

matter_t::data_t
matter_t::get_data(const vec3_t & eye_position){
    vec3_t r = sdf->get_aabb().get_size();
    vec3_t eye = transform.to_local_space(eye_position);

    float near = (eye.abs() - r).max(vec3_t()).norm();
    float far  = (eye.abs() + r).norm();

    return data_t(
        near, far,
        r.cast<float>(),
        id,
        *transform.get_matrix()
    );
}

vec3_t 
matter_t::get_position() const {
    return transform.get_position();
}

void 
matter_t::set_position(const vec3_t & x){
    transform.set_position(x);
}

void 
matter_t::set_rotation(const quat_t & q){
    transform.set_rotation(q);
}

double 
matter_t::phi(const vec3_t & x) const {
    return sdf->phi(transform.to_local_space(x));
}

uint32_t 
matter_t::get_id() const {
    return id;
}